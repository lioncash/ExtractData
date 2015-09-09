#include "stdafx.h"
#include "../ExtractBase.h"
#include "../bzip2/bzlib.h"
#include "../Image.h"
#include "Standard.h"
#include "Nscr.h"

/// Mounting
///
/// @param pclArc Archive
///
BOOL CNscr::Mount(CArcFile* pclArc)
{
	if (MountNsa(pclArc))
		return TRUE;

	if (MountSar(pclArc))
		return TRUE;

	if (MountScr(pclArc))
		return TRUE;

	return FALSE;
}

/// nsa mounting
///
/// @param pclArc Archive
///
BOOL CNscr::MountNsa(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	if (pclArc->GetArcExten() != _T(".nsa"))
	{
		return FALSE;
	}

	// Get file count
	DWORD dwFiles = 0;

	if (memcmp(pclArc->GetHed(), "\0\0", 2) == 0)
	{
		// Each 4 bytes

		pclArc->Read(&dwFiles, 4);
	}
	else
	{
		pclArc->Read(&dwFiles, 2);
		dwFiles <<= 16;
	}

	pclArc->ConvEndian(&dwFiles);

	// Get offset
	DWORD dwOffset;

	pclArc->Read(&dwOffset, 4);
	pclArc->ConvEndian(&dwOffset);

	if (memcmp(pclArc->GetHed(), "\0\0", 2) == 0)
	{
		dwOffset += 2;
	}

	// Get index size
	DWORD dwIndexSize = dwOffset;
	dwIndexSize -= (memcmp(pclArc->GetHed(), "\0\0", 2) == 0) ? 8 : 6;

	// Get the index
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	DWORD dwIndexPtr = 0;

	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	for (DWORD i = 0; i < dwFiles; i++)
	{
		// Get file name
		TCHAR szFileName[256];
		lstrcpy(szFileName, (LPCTSTR)&clmbtIndex[dwIndexPtr]);
		dwIndexPtr += lstrlen(szFileName) + 1;

		BYTE btType = clmbtIndex[dwIndexPtr];

		// Add to list view
		SFileInfo stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.start = pclArc->ConvEndian(*(DWORD*)&clmbtIndex[dwIndexPtr + 1]) + dwOffset;
		stFileInfo.sizeCmp = pclArc->ConvEndian(*(DWORD*)&clmbtIndex[dwIndexPtr + 5]);
		stFileInfo.sizeOrg = pclArc->ConvEndian(*(DWORD*)&clmbtIndex[dwIndexPtr + 9]);
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		switch (btType)
		{
		case 1:
			stFileInfo.format = _T("SPB");
			break;

		case 2:
			stFileInfo.format = _T("LZSS");
			break;
		}

		pclArc->AddFileInfo(stFileInfo);

		dwIndexPtr += 13;
	}

	return TRUE;
}

/// sar mounting
///
/// @param pclArc Archive
///
BOOL CNscr::MountSar(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".sar"))
		return FALSE;

	// Get file count
	WORD dwFiles;
	pclArc->Read(&dwFiles, 2);
	pclArc->ConvEndian(&dwFiles);

	// Get offset
	DWORD dwOffset;
	pclArc->Read(&dwOffset, 4);
	pclArc->ConvEndian(&dwOffset);

	// Get index size
	DWORD dwIndexSize = dwOffset - 6;

	// Get the index
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	DWORD dwIndexPtr = 0;

	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	for (DWORD i = 0; i < dwFiles; i++)
	{
		// Get file name
		TCHAR szFileName[256];
		lstrcpy(szFileName, (LPCTSTR)&clmbtIndex[dwIndexPtr]);
		dwIndexPtr += lstrlen(szFileName) + 1;

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.start = pclArc->ConvEndian(*(DWORD*)&clmbtIndex[dwIndexPtr + 0]) + dwOffset;
		stFileInfo.sizeOrg = pclArc->ConvEndian(*(DWORD*)&clmbtIndex[dwIndexPtr + 4]);
		stFileInfo.sizeCmp = stFileInfo.sizeOrg;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo(stFileInfo);

		dwIndexPtr += 8;
	}

	return TRUE;
}

/// Script file mounting
///
/// @param pclArc Archive
///
BOOL CNscr::MountScr(CArcFile* pclArc)
{
	if (pclArc->GetArcName() != _T("nscript.dat"))
		return FALSE;

	return pclArc->Mount();
}

/// Decoding
///
/// @param pclArc Archive
///
BOOL CNscr::Decode(CArcFile* pclArc)
{
	if (DecodeScr(pclArc))
		return TRUE;

	if (DecodeSPB(pclArc))
		return TRUE;

	if (DecodeLZSS(pclArc))
		return TRUE;

	if (DecodeNBZ(pclArc))
		return TRUE;

	return FALSE;
}

/// Script file decoding
///
/// @param pclArc Archive
///
BOOL CNscr::DecodeScr(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	if (pstFileInfo->name != _T("nscript.dat"))
		return FALSE;

	// Ensure buffer exists
	DWORD dwBufferSize = pclArc->GetBufSize();
	YCMemory<BYTE> clmbtBuffer(dwBufferSize);
	YCMemory<BYTE> clmbtBuffer2(dwBufferSize * 2);

	// Generate output files

	pclArc->OpenFile(_T(".txt"));

	for (DWORD dwWriteSize = 0; dwWriteSize != pstFileInfo->sizeOrg; dwWriteSize += dwBufferSize)
	{
		// Buffer adjustment
		pclArc->SetBufSize(&dwBufferSize, dwWriteSize);

		// Reading
		pclArc->Read(&clmbtBuffer[0], dwBufferSize);

		DWORD dwBufferSize2 = 0;

		for (DWORD i = 0; i < dwBufferSize; i++)
		{
			// Decoding
			clmbtBuffer2[dwBufferSize2] = clmbtBuffer[i] ^ 0x84;

			// Change to CR + LF line endings
			if (clmbtBuffer2[dwBufferSize2] == '\n')
			{
				clmbtBuffer2[dwBufferSize2++] = '\r';
				clmbtBuffer2[dwBufferSize2] = '\n';
			}

			dwBufferSize2++;
		}

		pclArc->WriteFile(&clmbtBuffer2[0], dwBufferSize2);
	}

	return TRUE;
}

/// NBZ Decoding
///
/// @param pclArc Archive
///
BOOL CNscr::DecodeNBZ(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	if (pstFileInfo->format != _T("NBZ"))
		return FALSE;

	// Get file size
	DWORD dwDstSize;
	pclArc->Read(&dwDstSize, 4);
	pclArc->ConvEndian(&dwDstSize);

	DWORD dwSrcSize = (pstFileInfo->sizeCmp - 4);

	// Ensure buffer exists
	YCMemory<BYTE> clmbtSrc(dwSrcSize);
	YCMemory<BYTE> clmbtDst(dwDstSize);

	// NBZ decompression
	pclArc->Read(&clmbtSrc[0], dwSrcSize);
	BZ2_bzBuffToBuffDecompress((char*)&clmbtDst[0], &(UINT&)dwDstSize, (char*)&clmbtSrc[0], dwSrcSize, 0, 0);

	// Obtain file extension
	YCString clsFileExt;
	GetFileExt(clsFileExt, &clmbtDst[0]);

	// Output
	pclArc->OpenFile(clsFileExt);
	pclArc->WriteFile(&clmbtDst[0], dwDstSize);

	return TRUE;
}

/// Getting file extensions
///
/// @param rfclsDst  Destination
/// @param pbtBuffer Buffer
///
void CNscr::GetFileExt(YCString& rfclsDst, const BYTE* pbtBuffer)
{
	if (memcmp(pbtBuffer, "BM", 2) == 0)
	{
		rfclsDst = _T(".bmp");
	}
	else if ((memcmp(pbtBuffer, "RIFF", 4) == 0) && (memcmp(&pbtBuffer[8], "WAVEfmt ", 8) == 0))
	{
		rfclsDst = _T(".wav");
	}
	else if (memcmp(pbtBuffer, "MThd", 4) == 0)
	{
		rfclsDst = _T(".mid");
	}
}

/// Get a stream of bits
///
/// @param pbtSrc            Input data
/// @param dwReadBitLength   Number of bits to read
/// @param pdwReadByteLength Number of bytes read
///
DWORD CNscr::GetBit(const BYTE* pbtSrc, DWORD dwReadBitLength, DWORD* pdwReadByteLength)
{
	DWORD dwResult = 0;
	DWORD dwSrcPtr = 0;

	for (DWORD i = 0; i < dwReadBitLength; i++)
	{
		if (btMaskForGetBit == 0)
		{
			btSrcForGetBit = pbtSrc[dwSrcPtr++];
			btMaskForGetBit = 0x80;
		}

		dwResult <<= 1;

		if (btSrcForGetBit & btMaskForGetBit)
		{
			dwResult++;
		}

		btMaskForGetBit >>= 1;
	}

	*pdwReadByteLength = dwSrcPtr;

	return dwResult;
}

/// SPB Decoding
///
/// @param pclArc Archive
///
BOOL CNscr::DecodeSPB(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	if (pstFileInfo->format != _T("SPB"))
		return FALSE;

	// Get the width
	WORD wWidth;
	pclArc->Read(&wWidth, 2);
	pclArc->ConvEndian(&wWidth);

	// Get the height
	WORD wHeight;
	pclArc->Read(&wHeight, 2);
	pclArc->ConvEndian(&wHeight);

	// Parameters for the image
	DWORD dwColorSize = (wWidth * wHeight);
	DWORD dwLine = (wWidth * 3);
	DWORD dwPitch = ((dwLine + 3) & 0xFFFFFFFC);

	// Ensure buffers exist
	DWORD dwSrcSize = pstFileInfo->sizeCmp - 4;
	DWORD dwDstSize = pstFileInfo->sizeOrg - 54;
	DWORD dwWorkSize = (dwColorSize + 4) * 3;

	YCMemory<BYTE> clmbtSrc(dwSrcSize);
	YCMemory<BYTE> clmbtDst(dwDstSize);
	YCMemory<BYTE> clmbtWork(dwWorkSize);

	// Read the SPB file
	pclArc->Read(&clmbtSrc[0], dwSrcSize);

	// SPB decompression
	DWORD dwSrcPtr = 0;
	DWORD dwDstPtr = 0;
	DWORD dwWorkPtr = 0;
	DWORD adwWorkPtrForSave[3];

	btMaskForGetBit = 0;
	btSrcForGetBit = 0;

	for (unsigned int i = 0; i < 3; i++)
	{
		DWORD dwReadByteLength;
		DWORD dwData;
		DWORD dwWork;

		adwWorkPtrForSave[i] = dwWorkPtr;

		dwData = GetBit(&clmbtSrc[dwSrcPtr], 8, &dwReadByteLength);

		dwSrcPtr += dwReadByteLength;

		clmbtWork[dwWorkPtr++] = static_cast<BYTE>(dwData);

		// Decompression

		for (DWORD dwCount = 1; dwCount < dwColorSize; dwCount += 4)
		{
			DWORD dwFlags = GetBit(&clmbtSrc[dwSrcPtr], 3, &dwReadByteLength);
			DWORD dwFlags2;

			dwSrcPtr += dwReadByteLength;

			switch (dwFlags)
			{
			case 0:
				clmbtWork[dwWorkPtr + 0] = static_cast<BYTE>(dwData);
				clmbtWork[dwWorkPtr + 1] = static_cast<BYTE>(dwData);
				clmbtWork[dwWorkPtr + 2] = static_cast<BYTE>(dwData);
				clmbtWork[dwWorkPtr + 3] = static_cast<BYTE>(dwData);

				dwWorkPtr += 4;
				continue;

			case 7:
				dwFlags2 = GetBit(&clmbtSrc[dwSrcPtr], 1, &dwReadByteLength) + 1;
				dwSrcPtr += dwReadByteLength;
				break;

			default:
				dwFlags2 = (dwFlags + 2);
				break;
			}

			for (unsigned int j = 0; j < 4; j++)
			{
				if (dwFlags2 == 8)
				{
					dwData = GetBit(&clmbtSrc[dwSrcPtr], 8, &dwReadByteLength);
				}
				else
				{
					dwWork = GetBit(&clmbtSrc[dwSrcPtr], dwFlags2, &dwReadByteLength);

					if (dwWork & 1)
					{
						dwData += (dwWork >> 1) + 1;
					}
					else
					{
						dwData -= (dwWork >> 1);
					}
				}

				dwSrcPtr += dwReadByteLength;
				clmbtWork[dwWorkPtr++] = static_cast<BYTE>(dwData);
			}
		}
	}

	// RGB synthesis

	BYTE* pbtDst = &clmbtDst[dwPitch * (wHeight - 1)];
	const BYTE* apbtWork[3];

	apbtWork[0] = &clmbtWork[adwWorkPtrForSave[0]];
	apbtWork[1] = &clmbtWork[adwWorkPtrForSave[1]];
	apbtWork[2] = &clmbtWork[adwWorkPtrForSave[2]];

	for (WORD j = 0; j < wHeight; j++)
	{
		if (j & 1)
		{
			// Odd line

			for (WORD k = 0; k < wWidth; k++)
			{
				pbtDst[0] = *apbtWork[0]++;
				pbtDst[1] = *apbtWork[1]++;
				pbtDst[2] = *apbtWork[2]++;

				pbtDst -= 3;
			}

			pbtDst -= (dwPitch - 3);
		}
		else
		{
			// Even line

			for (WORD k = 0; k < wWidth; k++)
			{
				pbtDst[0] = *apbtWork[0]++;
				pbtDst[1] = *apbtWork[1]++;
				pbtDst[2] = *apbtWork[2]++;

				pbtDst += 3;
			}

			pbtDst -= (dwPitch + 3);
		}
	}

	// Output
	CImage clImage;
	clImage.Init(pclArc, wWidth, wHeight, 24);
	clImage.Write(&clmbtDst[0], dwDstSize);

	return TRUE;
}

/// LZSS Decoding
///
/// @param pclArc Archive
///
BOOL CNscr::DecodeLZSS(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	if (pstFileInfo->format != _T("LZSS"))
		return FALSE;

	// Ensure buffers exist
	DWORD dwSrcSize = pstFileInfo->sizeCmp;
	DWORD dwDstSize = pstFileInfo->sizeOrg;
	DWORD dwDicSize = 256;

	YCMemory<BYTE> clmbtSrc(dwSrcSize);
	YCMemory<BYTE> clmbtDst(dwDstSize);
	YCMemory<BYTE> clmbtDic(dwDicSize);

	ZeroMemory(&clmbtDic[0], dwDicSize);

	// Read
	pclArc->Read(&clmbtSrc[0], dwSrcSize);

	// LZSS decompression
	DWORD dwSrcPtr = 0;
	DWORD dwDstPtr = 0;
	DWORD dwDicPtr = 239;

	btMaskForGetBit = 0;
	btSrcForGetBit = 0;

	while (dwDstPtr < dwDstSize)
	{
		DWORD dwReadByteLength;
		DWORD dwData;

		// Get the compression flag
		DWORD dwFlag = GetBit(&clmbtSrc[dwSrcPtr], 1, &dwReadByteLength);

		dwSrcPtr += dwReadByteLength;

		// Uncompressed
		if (dwFlag & 1)
		{
			dwData = GetBit(&clmbtSrc[dwSrcPtr], 8, &dwReadByteLength);

			clmbtDst[dwDstPtr++] = static_cast<BYTE>(dwData);
			clmbtDic[dwDicPtr++] = static_cast<BYTE>(dwData);

			dwSrcPtr += dwReadByteLength;
			dwDicPtr &= (dwDicSize - 1);
		}
		else // Compressed
		{
			DWORD dwBack = GetBit(&clmbtSrc[dwSrcPtr], 8, &dwReadByteLength);
			dwSrcPtr += dwReadByteLength;

			DWORD dwLength = GetBit(&clmbtSrc[dwSrcPtr], 4, &dwReadByteLength) + 2;
			dwSrcPtr += dwReadByteLength;

			if ((dwDstPtr + dwLength) > dwDstSize)
			{
				// Larger than the output buffer

				dwLength = (dwDstSize - dwDstPtr);
			}

			for (DWORD i = 0; i < dwLength; i++)
			{
				clmbtDst[dwDstPtr++] = clmbtDic[dwDicPtr++] = clmbtDic[dwBack++];

				dwDicPtr &= (dwDicSize - 1);
				dwBack &= (dwDicSize - 1);
			}
		}
	}

	// Output
	CImage clImage;
	clImage.Init(pclArc, &clmbtDst[0]);
	clImage.Write(dwDstSize);

	return TRUE;
}
