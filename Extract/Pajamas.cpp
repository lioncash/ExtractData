#include "stdafx.h"
#include "../Image.h"
#include "Pajamas.h"

/// Mounting
///
/// @param pclArc Archive
///
bool CPajamas::Mount(CArcFile* pclArc)
{
	if (MountDat1(pclArc))
		return true;

	if (MountDat2(pclArc))
		return true;

	return false;
}

/// GAMEDAT PACK Mounting
///
/// @param pclArc Archive
///
bool CPajamas::MountDat1(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".dat")) && (pclArc->GetArcExten() != _T(".pak")))
		return false;

	if (memcmp(pclArc->GetHed(), "GAMEDAT PACK", 12) != 0)
		return false;

	// Get file count
	DWORD dwFiles;
	pclArc->SeekHed(12);
	pclArc->Read(&dwFiles, 4);

	// Get index size from file count
	DWORD dwIndexSize = dwFiles * 24;

	// Get index
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	DWORD          dwIndexPtr = 0;
	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// Get index of the file information
	DWORD dwFileNameIndexSize = dwFiles << 4;
	BYTE* pbtFileInfoIndex = &clmbtIndex[dwFileNameIndexSize];

	// Get offset (Required for correction when the starting address is zero-based)
	DWORD dwOffset = 16 + dwIndexSize;

	for (DWORD i = 0; i < dwFiles; i++)
	{
		// Get filename
		TCHAR szFileName[16];
		memcpy(szFileName, &clmbtIndex[dwIndexPtr], 16);

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.start = *(DWORD*)&pbtFileInfoIndex[0] + dwOffset;
		stFileInfo.sizeCmp = *(DWORD*)&pbtFileInfoIndex[4];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo(stFileInfo);

		dwIndexPtr += 16;
		pbtFileInfoIndex += 8;
	}

	return true;
}

// GAMEDAT PAC2 Mounting
///
/// @param pclArc Archive
///
bool CPajamas::MountDat2(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".dat"))
		return false;

	if (memcmp(pclArc->GetHed(), "GAMEDAT PAC2", 12) != 0)
		return false;

	// Get file count
	DWORD dwFiles;
	pclArc->SeekHed(12);
	pclArc->Read(&dwFiles, 4);

	// Get index size from file count
	DWORD dwIndexSize = dwFiles * 40;

	// Get index
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	DWORD         dwIndexPtr = 0;
	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// Get index of the file information
	DWORD dwFileNameIndexSize = dwFiles << 5;
	BYTE* pbtFileInfoIndex = &clmbtIndex[dwFileNameIndexSize];

	// Get offset (Required for correction when the starting address is zero-based)
	DWORD dwOffset = 16 + dwIndexSize;

	for (DWORD i = 0; i < dwFiles; i++)
	{
		// Get filename
		TCHAR szFileName[32];
		memcpy(szFileName, &clmbtIndex[dwIndexPtr], 32);

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.start = *(DWORD*)&pbtFileInfoIndex[0] + dwOffset;
		stFileInfo.sizeCmp = *(DWORD*)&pbtFileInfoIndex[4];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo(stFileInfo);

		dwIndexPtr += 32;
		pbtFileInfoIndex += 8;
	}

	return true;
}

/// Decoding
///
/// @param pclArc Archive
///
bool CPajamas::Decode(CArcFile* pclArc)
{
	if (DecodeEPA(pclArc))
		return true;

	return false;
}

/// EPA Decoding
///
/// @param pclArc Archive
///
bool CPajamas::DecodeEPA(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();
	if (file_info->format != _T("EPA"))
		return false;

	// Read header
	BYTE abtHeader[16];
	pclArc->Read(abtHeader, sizeof(abtHeader));

	// Difference flag, Number of colors, width, height
	BYTE btDiffFlag = abtHeader[3];
	WORD wBpp = abtHeader[4];
	long lWidth = *(long*)&abtHeader[8];
	long lHeight = *(long*)&abtHeader[12];

	switch (wBpp)
	{
	case 0:
		wBpp = 8;
		break;

	case 1:
		wBpp = 24;
		break;

	case 2:
		wBpp = 32;
		break;

	case 4:
		wBpp = 8;
		break;
	}

	switch (btDiffFlag)
	{
	case 1: // Usual
		break;

	case 2: // Difference
		pclArc->SeekCur(40);
		break;

	default: // Unknown
		pclArc->SeekHed(file_info->start);
		return false;
	}

	// Read palette
	BYTE abtPallet[768];
	if (wBpp == 8)
	{
		pclArc->Read(abtPallet, sizeof(abtPallet));
	}

	// Read EPA data
	DWORD dwSrcSize = file_info->sizeCmp - 16;
	if (wBpp == 8)
	{
		dwSrcSize -= sizeof(abtPallet);
	}
	YCMemory<BYTE> clmbtSrc(dwSrcSize);
	pclArc->Read(&clmbtSrc[0], dwSrcSize);

	// Secure area to store the BMP data
	DWORD dwDstSize = lWidth * lHeight * (wBpp >> 3);
	YCMemory<BYTE> clmbtDst(dwDstSize);
	ZeroMemory(&clmbtDst[0], dwDstSize);

	// Decompress EPA
	DecompEPA(&clmbtDst[0], dwDstSize, &clmbtSrc[0], dwSrcSize, lWidth);

	// Output
	CImage clImage;
	clImage.Init(pclArc, lWidth, lHeight, wBpp, abtPallet, sizeof(abtPallet));
	clImage.WriteCompoBGRAReverse(&clmbtDst[0], dwDstSize);

	return true;
}

/// EPA Decompression
///
/// @param pvDst     Destination
/// @param dwDstSize Destination size
/// @param pvSrc     Compressed data
/// @param dwSrcSize Compressed data size
/// @param lWidth    Width
///
bool CPajamas::DecompEPA(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, long lWidth)
{
	const BYTE* pbtSrc = (const BYTE*)pvSrc;
	BYTE*       pbtDst = (BYTE*)pvDst;

	DWORD dwSrcPtr = 0;
	DWORD dwDstPtr = 0;

	// Offset table
	const DWORD adwOffsets[16] =
	{
		0, 1, lWidth, lWidth + 1,
		2, lWidth - 1, lWidth << 1, 3,
		(lWidth << 1) + 2, lWidth + 2, (lWidth << 1) + 1, (lWidth << 1) - 1,
		(lWidth << 1) - 2, lWidth - 2, lWidth * 3, 4
	};

	// Decompression
	while ((dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize))
	{
		BYTE btCode = pbtSrc[dwSrcPtr++];
		DWORD dwLength = btCode & 0x07;

		if (btCode & 0xF0)
		{
			if (btCode & 0x08)
			{
				dwLength = (dwLength << 8) + pbtSrc[dwSrcPtr++];
			}

			if (dwLength != 0)
			{
				btCode >>= 4;

				DWORD dwBack = dwDstPtr - adwOffsets[btCode];

				if ((dwDstPtr + dwLength) > dwDstSize)
				{
					// Exceeds output buffer
					dwLength = (dwDstSize - dwDstPtr);
				}

				for (DWORD i = 0; i < dwLength; i++)
				{
					pbtDst[dwDstPtr + i] = pbtDst[dwBack + i];
				}

				dwDstPtr += dwLength;
			}
		}
		else if (btCode != 0)
		{
			memcpy(&pbtDst[dwDstPtr], &pbtSrc[dwSrcPtr], btCode);

			dwSrcPtr += btCode;
			dwDstPtr += btCode;
		}
	}

	return true;
}
