#include "stdafx.h"
#include "../Image.h"
#include "AOS.h"

/// Mounting
///
/// @param pclArc Archive
///
BOOL CAOS::Mount(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".aos"))
		return FALSE;

	// Unknown 4 bytes
	pclArc->SeekHed(4);

	// Get offset
	DWORD dwOffset;
	pclArc->Read(&dwOffset, 4);

	// Get index size
	DWORD dwIndexSize;
	pclArc->Read(&dwIndexSize, 4);

	// Get archive filename
	char szArchiveName[261];
	pclArc->Read(szArchiveName, 261);
	if (pclArc->GetArcName() != szArchiveName)
	{
		// Archive filename is different

		pclArc->SeekHed();
		return FALSE;
	}

	// Get index
	YCMemory<BYTE> clmIndex(dwIndexSize);
	pclArc->Read(&clmIndex[0], dwIndexSize);

	// Get file info
	for (DWORD i = 0; i < dwIndexSize; i += 40)
	{
		SFileInfo stFileInfo;
		stFileInfo.name.Copy((char*)&clmIndex[i], 32);
		stFileInfo.start = *(DWORD*) &clmIndex[i + 32] + dwOffset;
		stFileInfo.sizeCmp = *(DWORD*) &clmIndex[i + 36];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo(stFileInfo);
	}

	return TRUE;
}

/// General decoding
///
/// @param pclArc Archive
///
BOOL CAOS::Decode(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".aos"))
		return FALSE;

	BOOL       bReturn = FALSE;
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	if (pstFileInfo->format == _T("ABM"))
	{
		// ABM
		bReturn = DecodeABM(pclArc);
	}
	else if (pstFileInfo->format == _T("MSK"))
	{
		// Image mask
		bReturn = DecodeMask( pclArc );
	}
	else if (pstFileInfo->format == _T("SCR"))
	{
		// Script
		bReturn = DecodeScript(pclArc);
	}

	return bReturn;
}


/// ABM Decoding
///
/// @param pclArc Archive
///
BOOL CAOS::DecodeABM(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	// Read data
	DWORD dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmSrc(dwSrcSize);
	pclArc->Read(&clmSrc[0], dwSrcSize);

	// Get bitmap header
	BITMAPFILEHEADER* pstbfhSrc = (BITMAPFILEHEADER*) &clmSrc[0];
	BITMAPINFOHEADER* pstbihSrc = (BITMAPINFOHEADER*) &clmSrc[14];

	CImage            clImage;
	YCMemory<BYTE>    clmDst;
	YCString          clsLastName;
	DWORD             dwDstSize;
	DWORD             dwFrames;
	DWORD             dwOffsetToData;
	DWORD             dwSrcPtr = 0;
	DWORD             dwDstPtr = 0;

	switch( pstbihSrc->biBitCount )
	{
	case 1: // Multi-frame
		dwFrames = *(DWORD*) &clmSrc[58];
		dwOffsetToData = *(DWORD*) &clmSrc[66];

		dwDstSize = (pstbihSrc->biWidth * pstbihSrc->biHeight * 4);
		clmDst.resize(dwDstSize);

		// Multiple files
		if (dwFrames >= 2)
		{
			clsLastName.Format(_T("_000"));
		}

		// Decompression
		dwSrcPtr = dwOffsetToData;
		for (DWORD i = 0; i < dwDstSize; i += 4)
		{
			clmDst[i + 0] = clmSrc[dwSrcPtr++];
			clmDst[i + 1] = clmSrc[dwSrcPtr++];
			clmDst[i + 2] = clmSrc[dwSrcPtr++];
			clmDst[i + 3] = 0xFF;
		}

		// Output
		clImage.Init(pclArc, pstbihSrc->biWidth, pstbihSrc->biHeight, 32, nullptr, 0, clsLastName);
		clImage.WriteReverse(&clmDst[0], dwDstSize);
		clImage.Close();

		// 

		for (DWORD i = 1; i < dwFrames; i++)
		{
			DWORD dwOffsetToFrame = *(DWORD*) &clmSrc[70 + (i - 1) * 4];
			clsLastName.Format(_T("_%03d"), i);

			// Decompression
			ZeroMemory(&clmDst[0], dwDstSize);
			DecompABM(&clmDst[0], dwDstSize, &clmSrc[dwOffsetToFrame], (dwSrcSize - dwOffsetToFrame));

			// Output
			clImage.Init(pclArc, pstbihSrc->biWidth, pstbihSrc->biHeight, 32, NULL, 0, clsLastName);
			clImage.WriteReverse(&clmDst[0], dwDstSize, FALSE);
			clImage.Close();
		}
		break;

	case 32: // 32bit

		dwDstSize = (pstbihSrc->biWidth * pstbihSrc->biHeight * 4);
		clmDst.resize(dwDstSize);

		// Decompression
		DecompABM(&clmDst[0], dwDstSize, &clmSrc[54], (dwSrcSize - 54));

		// Output
		clImage.Init(pclArc, pstbihSrc->biWidth, pstbihSrc->biHeight, pstbihSrc->biBitCount);
		clImage.WriteReverse(&clmDst[0], dwDstSize);
		clImage.Close();
		break;

	default: // Other
		pclArc->OpenFile();
		pclArc->WriteFile(&clmSrc[0], dwSrcSize);
		pclArc->CloseFile();
	}

	return TRUE;
}

/// Decode Image Mask
///
/// @param pclArc Archive
///
BOOL CAOS::DecodeMask(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	// Read Data
	DWORD          dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmSrc(dwSrcSize);
	pclArc->Read(&clmSrc[0], dwSrcSize);

	// Output
	CImage clImage;
	clImage.Init(pclArc, &clmSrc[0]);
	clImage.Write(dwSrcSize);
	clImage.Close();

	return TRUE;
}

/// Decode Script
///
/// @param pclArc Archive
///
BOOL CAOS::DecodeScript(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	// Read compressed data
	DWORD dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmSrc(dwSrcSize);
	pclArc->Read(&clmSrc[0], dwSrcSize);

	// Buffer allocation for extraction
	DWORD dwDstSize = *(DWORD*) &clmSrc[0];
	YCMemory<BYTE> clmDst(dwDstSize);

	// Decompression
	DecompScript(&clmDst[0], dwDstSize, &clmSrc[4], (dwSrcSize - 4));

	// Output
	pclArc->OpenScriptFile();
	pclArc->WriteFile(&clmDst[0], dwDstSize, dwSrcSize);
	pclArc->CloseFile();

	return TRUE;
}

/// ABM Decompression
///
/// @param pbtDst    Destination
/// @param dwDstSize Destination size
/// @param pbtSrc    Compressed data
/// @param dwSrcSize Compressed data size
///
BOOL CAOS::DecompABM(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize)
{
	DWORD dwSrcPtr = 0;
	DWORD dwDstPtr = 0;
	DWORD dwAlphaCount = 0;

	while( dwDstPtr < dwDstSize )
	{
		BYTE btCurrentSrc = pbtSrc[dwSrcPtr++];

		switch( btCurrentSrc )
		{
		case 0: // Is 0x00
		{
			DWORD dwLength = pbtSrc[dwSrcPtr++];

			for( DWORD i = 0 ; i < dwLength ; i++ )
			{
				pbtDst[dwDstPtr++] = 0;

				dwAlphaCount++;

				if( dwAlphaCount == 3 )
				{
					pbtDst[dwDstPtr++] = 0;

					dwAlphaCount = 0;
				}
			}

			break;
		}

		case 255: // Is 0xFF
		{

			DWORD dwLength = pbtSrc[dwSrcPtr++];

			for( DWORD i = 0 ; i < dwLength ; i++ )
			{
				pbtDst[dwDstPtr++] = pbtSrc[dwSrcPtr++];

				dwAlphaCount++;

				if( dwAlphaCount == 3 )
				{
					pbtDst[dwDstPtr++] = 0xFF;

					dwAlphaCount = 0;
				}
			}
			break;
		}

		default: // Other
			pbtDst[dwDstPtr++] = pbtSrc[dwSrcPtr++];
			dwAlphaCount++;

			if( dwAlphaCount == 3 )
			{
				pbtDst[dwDstPtr++] = btCurrentSrc;

				dwAlphaCount = 0;
			}
		}
	}

	return TRUE;
}

/// Decompress Script
///
/// @param pbtDst    Destination
/// @param dwDstSize Destination size
/// @param pbtSrc    Compressed data
/// @param dwSrcSize Compressed data size
///
BOOL CAOS::DecompScript(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize)
{
	// Construct huffman table
	DWORD adwTableOfBit0[511];
	DWORD adwTableOfBit1[511];
	DWORD dwSrcPtr = 0;
	DWORD dwTablePtr = 256;
	DWORD dwCurrentSrc = 0;
	DWORD dwBitShift = 0;

	ZeroMemory(adwTableOfBit0, sizeof(adwTableOfBit0));
	ZeroMemory(adwTableOfBit1, sizeof(adwTableOfBit1));

	dwTablePtr = CreateHuffmanTable(adwTableOfBit0, adwTableOfBit1, pbtSrc, &dwSrcPtr, &dwTablePtr, &dwCurrentSrc, &dwBitShift);

	// Decompress
	DecompHuffman(pbtDst, dwDstSize, adwTableOfBit0, adwTableOfBit1, &pbtSrc[dwSrcPtr], dwTablePtr, dwCurrentSrc, dwBitShift);

	return TRUE;
}

/// Construct Huffman Table
///
/// @param pdwTableOfBit0 bit0 table
/// @param pdwTableOfBit1 bit1 table
/// @param pbtSrc         Compressed data
/// @param pdwSrcPtr      Compressed data pointer
/// @param pdwTablePtr    Table pointer
/// @param pdwCurrentSrc  Current data
/// @param pdwBitShift    Bit shift
///
DWORD CAOS::CreateHuffmanTable(DWORD* pdwTableOfBit0, DWORD* pdwTableOfBit1, const BYTE* pbtSrc, DWORD* pdwSrcPtr, DWORD* pdwTablePtr, DWORD* pdwCurrentSrc, DWORD* pdwBitShift)
{
	DWORD dwReturn = 0;
	DWORD dwTablePtr;

	if (*pdwBitShift == 0)
	{
		// Read 8-bits

		*pdwCurrentSrc = pbtSrc[(*pdwSrcPtr)++];
		*pdwBitShift = 8;
	}

	(*pdwBitShift) -= 1;

	// Bit 1
	if ((*pdwCurrentSrc >> *pdwBitShift) & 1)
	{
		dwTablePtr = *pdwTablePtr;

		(*pdwTablePtr) += 1;

		if (dwTablePtr < 511)
		{
			pdwTableOfBit0[dwTablePtr] = CreateHuffmanTable(pdwTableOfBit0, pdwTableOfBit1, pbtSrc, pdwSrcPtr, pdwTablePtr, pdwCurrentSrc, pdwBitShift);
			pdwTableOfBit1[dwTablePtr] = CreateHuffmanTable(pdwTableOfBit0, pdwTableOfBit1, pbtSrc, pdwSrcPtr, pdwTablePtr, pdwCurrentSrc, pdwBitShift);

			dwReturn = dwTablePtr;
		}
	}
	else // Bit 0
	{
		DWORD dwBitShiftTemp = 8;
		DWORD dwResult = 0;

		while (dwBitShiftTemp > *pdwBitShift)
		{
			DWORD dwWork = ((1 << *pdwBitShift) - 1) & *pdwCurrentSrc;

			dwBitShiftTemp -= *pdwBitShift;

			*pdwCurrentSrc = pbtSrc[(*pdwSrcPtr)++];

			dwResult |= (dwWork << dwBitShiftTemp);

			*pdwBitShift = 8;
		}
		(*pdwBitShift) -= dwBitShiftTemp;

		DWORD dwMask = (1 << dwBitShiftTemp) - 1;
		dwReturn = ((*pdwCurrentSrc >> *pdwBitShift) & dwMask) | dwResult;
	}

	return dwReturn;
}

/// Huffman Decompression
///
/// @param pbtDst         Destination
/// @param dwDstSize      Destination size
/// @param pdwTableOfBit0 bit0 table
/// @param pdwTableOfBit1 bit1 table
/// @param pbtSrc         Compressed data
/// @param dwRoot         Table position reference
/// @param dwCurrentSrc   Current data
/// @param dwBitShift     Bit shift
///
BOOL CAOS::DecompHuffman(BYTE* pbtDst, DWORD dwDstSize, const DWORD* pdwTableOfBit0, const DWORD* pdwTableOfBit1, const BYTE* pbtSrc, DWORD dwRoot, DWORD dwCurrentSrc, DWORD dwBitShift)
{
	if (dwDstSize <= 0)
		return FALSE;

	DWORD dwSrcPtr = 0;
	DWORD dwDstPtr = 0;

	while (dwDstPtr < dwDstSize)
	{
		DWORD dwTablePtr = dwRoot;

		while (dwTablePtr >= 256)
		{
			if (dwBitShift == 0)
			{
				// Read 8-bits
				dwCurrentSrc = pbtSrc[dwSrcPtr++];
				dwBitShift = 8;
			}

			dwBitShift -= 1;

			// bit 1
			if ((dwCurrentSrc >> dwBitShift) & 1)
			{
				dwTablePtr = pdwTableOfBit1[dwTablePtr];
			}
			else // bit 0
			{
				dwTablePtr = pdwTableOfBit0[dwTablePtr];
			}
		}

		pbtDst[dwDstPtr++] = (BYTE) dwTablePtr;
	}

	return TRUE;
}
