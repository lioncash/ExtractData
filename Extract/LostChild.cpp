#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Arc/LZSS.h"
#include "../Image.h"
#include "LostChild.h"

/// Mounting
///
/// @param pclArc Archive
///
BOOL CLostChild::Mount(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "EPK ", 4) != 0)
		return FALSE;

	pclArc->SeekHed(4);

	// Get index size
	DWORD dwIndexSize;
	pclArc->Read(&dwIndexSize, 4);
	dwIndexSize -= 32;
	pclArc->SeekCur(16);

	// Get file count
	DWORD dwFiles;
	pclArc->Read(&dwFiles, 4);
	pclArc->SeekCur(4);

	// Get index
	YCMemory<BYTE> clmIndex(dwIndexSize);
	pclArc->Read(&clmIndex[0], dwIndexSize);

	// Get the filename index
	BYTE* pbtFileNameIndex = &clmIndex[*(DWORD*)&clmIndex[8]] - 32;

	// Split archive files
	if (pclArc->GetArcName() == _T("data.epk"))
	{
		YCString clsPathToArc = pclArc->GetArcPath();

		for (unsigned int i = 1; i <= 3; i++)
		{
			YCString clsArcExt;
			clsArcExt.Format(_T(".e%02d"), i);
			clsPathToArc.RenameExtension(clsArcExt);
			pclArc->Open(clsPathToArc);
			pclArc->GetProg()->ReplaceAllFileSize(pclArc->GetArcSize());
		}

		pclArc->SetFirstArc();
	}

	// Get file information
	DWORD dwIndexPtr = 0;
	DWORD dwFileNameIndexPtr = 0;

	for (DWORD i = 0; i < dwFiles; i++)
	{
		// Get filename
		char  szFileName[_MAX_FNAME];
		DWORD dwLength = *(DWORD*)&pbtFileNameIndex[dwFileNameIndexPtr];

		for (DWORD j = 0; j < dwLength; j++)
		{
			szFileName[j] = pbtFileNameIndex[dwFileNameIndexPtr + 4 + j] ^ 0xFF;
		}

		szFileName[dwLength] = pbtFileNameIndex[dwFileNameIndexPtr + 4 + dwLength];
		dwFileNameIndexPtr += 4 + dwLength + 1;

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.start = *(UINT64*)&clmIndex[dwIndexPtr + 16];
		stFileInfo.sizeCmp = *(DWORD*)&clmIndex[dwIndexPtr + 24];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.title = _T("LOST CHILD");

		dwIndexPtr += 40;

		if (stFileInfo.start > 3900000000)
		{
			stFileInfo.start -= 3900000000;

			if (pclArc->GetArcsID() == 2)
			{
				pclArc->SetNextArc();
			}
		}
		else if (stFileInfo.start > 2600000000)
		{
			stFileInfo.start -= 2600000000;

			if (pclArc->GetArcsID() == 1)
			{
				pclArc->SetNextArc();
			}
		}
		else if (stFileInfo.start > 1300000000)
		{
			stFileInfo.start -= 1300000000;

			if (pclArc->GetArcsID() == 0)
			{
				pclArc->SetNextArc();
			}
		}

		// File size adjustment exceeds 1.2GB when adding
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;
		if (stFileInfo.end > 1300000000)
		{
			stFileInfo.end -= 1300000000;
		}

		pclArc->AddFileInfo(stFileInfo);
	}

	pclArc->SetFirstArc();

	return TRUE;
}

/// Decoding
///
/// @param pclArc Archive
///
BOOL CLostChild::Decode(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	if (pstFileInfo->title != _T("LOST CHILD"))
		return FALSE;

	if (DecodeESUR(pclArc))
		return TRUE;

	if (DecodeLAD(pclArc))
		return TRUE;

	return Extract(pclArc);
}

/// ESUR Decoding
///
/// @param pclArc Archive
///
bool CLostChild::DecodeESUR(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	if (pstFileInfo->format != _T("SUR"))
		return false;

	// Read
	DWORD dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmSrc(dwSrcSize);
	DWORD dwReadSize = pclArc->Read(&clmSrc[0], dwSrcSize);

	if (dwReadSize < dwSrcSize)
	{
		pclArc->SetNextArc();
		pclArc->SeekHed();

		pclArc->Read(&clmSrc[dwReadSize], (dwSrcSize - dwReadSize));
	}

	// Get header information
	long  lWidth = *(long*)&clmSrc[8];
	long  lHeight = *(long*)&clmSrc[12];
	DWORD dwDstSize = *(DWORD*)&clmSrc[4] - 32;
	WORD  wBpp = 32;

	// Get a buffer for LZSS decompression
	YCMemory<BYTE> clmDst(dwDstSize);

	// LZSS Decompression
	DecompLZSS(&clmDst[0], dwDstSize, &clmSrc[32], (dwSrcSize - 32), 4096, 4078, 3);

	// Output
	CImage clImage;
	clImage.Init(pclArc, lWidth, lHeight, wBpp);
	clImage.WriteReverse(&clmDst[0], dwDstSize);
	clImage.Close();

	return true;
}

/// LAD Decoding
///
/// @param pclArc Archive
///
bool CLostChild::DecodeLAD(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	if (pstFileInfo->format != _T("LAD"))
		return false;

	// Reading
	DWORD dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmSrc(dwSrcSize);
	DWORD dwReadSize = pclArc->Read(&clmSrc[0], dwSrcSize);

	if (dwReadSize < dwSrcSize)
	{
		pclArc->SetNextArc();
		pclArc->SeekHed();

		pclArc->Read(&clmSrc[dwReadSize], (dwSrcSize - dwReadSize));
	}

	// Get header info
	long  lWidth = *(long*)&clmSrc[8];
	long  lHeight = *(long*)&clmSrc[12];
	DWORD dwDstSize = *(DWORD*)&clmSrc[28];
	WORD  wBpp = 8;

	// Get a buffer for LZSS decoding
	YCMemory<BYTE> clmDst(dwDstSize);

	// LZSS Decompression
	DecompLZSS(&clmDst[0], dwDstSize, &clmSrc[32], (dwSrcSize - 32), 4096, 4078, 3);

	pclArc->OpenFile();
	pclArc->WriteFile(&clmDst[0], dwDstSize, dwSrcSize);
	pclArc->CloseFile();

	// Output
//	CImage clImage;
//	clImage.Init(pclArc, lWidth, lHeight, wBpp);
//	clImage.WriteReverse(&clmDst[0], dwDstSize);
//	clImage.Close();

	return true;
}

/// LZSS Decompression
///
/// @param pvDst          Destination
/// @param dwDstSize      Destination size
/// @param pvSrc          Input data
/// @param dwSrcSize      Input data size
/// @param dwDicSize      Dictionary size
/// @param dwDicPtr       Dictionary reference position
/// @param dwLengthOffset Length offset
///
bool CLostChild::DecompLZSS(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, DWORD dwDicSize, DWORD dwDicPtr, DWORD dwLengthOffset)
{
	BYTE*       pbtDst = (BYTE*)pvDst;
	const BYTE* pbtSrc = (const BYTE*)pvSrc;

	// Allocate buffer
	YCMemory<BYTE> clmbtDic(dwDicSize);
	ZeroMemory(&clmbtDic[0], dwDicSize);

	// Decoding
	DWORD dwSrcPtr = 0;
	DWORD dwDstPtr = 0;
	BYTE  btFlags = 0;
	DWORD dwBitCount = 0;

	while ((dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize))
	{
		if (dwBitCount == 0)
		{
			// 8bit reading
			btFlags = pbtSrc[dwSrcPtr++];
			dwBitCount = 8;
		}

		if (btFlags & 1)
		{
			// Uncompressed data

			pbtDst[dwDstPtr] = clmbtDic[dwDicPtr] = pbtSrc[dwSrcPtr];

			dwDstPtr++;
			dwSrcPtr++;
			dwDicPtr++;

			dwDicPtr &= (dwDicSize - 1);
		}
		else
		{
			// Compressed data

			BYTE  btLow = pbtSrc[dwSrcPtr++];
			BYTE  btHigh = pbtSrc[dwSrcPtr++];

			DWORD dwBack = ((btLow << 4) | (btHigh >> 4));
			DWORD dwLength = ((btHigh & 0x0F) + dwLengthOffset);

			if ((dwDstPtr + dwLength) > dwDstSize)
			{
				// Exceeds the output buffer

				dwLength = (dwDstSize - dwDstPtr);
			}

			for (DWORD j = 0; j < dwLength; j++)
			{
				pbtDst[dwDstPtr] = clmbtDic[dwDicPtr] = clmbtDic[dwBack];

				dwDstPtr++;
				dwDicPtr++;
				dwBack++;

				dwDicPtr &= (dwDicSize - 1);
				dwBack &= (dwDicSize - 1);
			}
		}

		btFlags >>= 1;
		dwBitCount--;
	}

	return true;
}

/// Extraction
///
/// @param pclArc Archive
///
BOOL CLostChild::Extract(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	if (pstFileInfo->title != _T("LOST CHILD"))
	{
		return FALSE;
	}

	// Ensure buffer exists
	DWORD dwBufferSize = pclArc->GetBufSize();
	YCMemory<BYTE> clmBuffer(dwBufferSize);

	// Generate output files
	pclArc->OpenFile();

	for (DWORD WriteSize = 0; WriteSize != pstFileInfo->sizeCmp; WriteSize += dwBufferSize)
	{
		// Adjust buffer size
		pclArc->SetBufSize(&dwBufferSize, WriteSize);

		// Read
		DWORD dwReadSize = pclArc->Read(&clmBuffer[0], dwBufferSize);
		pclArc->WriteFile(&clmBuffer[0], dwReadSize);

		if (dwReadSize < dwBufferSize)
		{
			pclArc->SetNextArc();

			pclArc->SeekHed();
			pclArc->Read(&clmBuffer[0], (dwBufferSize - dwReadSize));
			pclArc->WriteFile(&clmBuffer[0], (dwBufferSize - dwReadSize));
		}
	}

	pclArc->CloseFile();

	return TRUE;
}
