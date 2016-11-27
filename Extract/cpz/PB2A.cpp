#include "StdAfx.h"
#include "Extract/cpz/PB2A.h"

#include "ArcFile.h"
#include "Extract/cpz/JBP1.h"
#include "Image.h"

/// Decode
///
/// Remark: pvSrc can be used to decrypt the first part
///
/// @param pclArc - Archive
/// @param pvSrc  - Compressed data
/// @param dwSize - Compressed data size
///
bool CPB2A::Decode(CArcFile* pclArc, void* pvSrc, DWORD dwSrcSize)
{
	BYTE* pbtSrc = (BYTE*)pvSrc;

	// Decrypt
	Decrypt(pbtSrc, dwSrcSize);

	// Get image information
	WORD wType = *(WORD*)&pbtSrc[16];
	long lWidth = *(WORD*)&pbtSrc[18];
	long lHeight = *(WORD*)&pbtSrc[20];
	WORD wBpp = *(WORD*)&pbtSrc[22];

	// Decompression
	switch (wType)
	{
	case 1:
		Decode1(pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp);
		break;

	case 2:
		Decode2(pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp);
		break;

	case 3:
	case 4:
		Decode4(pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp);
		break;

	case 5:
		Decode5(pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp);
		break;

	case 6:
		Decode6(pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp);
		break;

	default: // Unknown
		pclArc->OpenFile();
		pclArc->WriteFile(pbtSrc, dwSrcSize);
		break;
	}

	return true;
}

///  Decoding
///
/// @param pbtTarget Data to be decoded
/// @param dwSize    Data size
///
void CPB2A::Decrypt(BYTE* pbtTarget, DWORD dwSize)
{
	for (int i = 8; i < 32; i += 2)
	{
		*(WORD*)&pbtTarget[i] ^= *(WORD*)&pbtTarget[dwSize - 3];
	}

	for (int i = 8, j = 0; i < 32; i++, j++)
	{
		pbtTarget[i] -= pbtTarget[dwSize - 27 + j];
	}
}

/// Decode 1
///
/// @param pclArc    Archive
/// @param pbtSrc    Compressed data
/// @param dwSrcSize Compressed data size
/// @param lWidth    Width
/// @param lHeight   Height
/// @param wBpp      Number of bits
///
bool CPB2A::Decode1(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp)
{
	// Ensure output buffer
	DWORD dwDstSize = (lWidth * lHeight * (wBpp >> 3));
	YCMemory<BYTE> clmbtDst( dwDstSize );

	// Decompression
	Decomp1( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );

	// Output
	CImage clImage;
	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmbtDst[0], dwDstSize );

	return true;
}

/// Decode 2
///
/// @param pclArc    Archive
/// @param pbtSrc    Compressed data
/// @param dwSrcSize Compressed data size
/// @param lWidth    Width
/// @param lHeight   Height
/// @param wBpp      Number of bits
///
bool CPB2A::Decode2(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp)
{
	// Ensure output buffer
	DWORD dwDstSize = (lWidth * lHeight * (wBpp >> 3));
	YCMemory<BYTE> clmbtDst(dwDstSize);

	// Decompression
	Decomp2(&clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp);

	// Output
	CImage clImage;
	clImage.Init(pclArc, lWidth, lHeight, wBpp);
	clImage.WriteReverse(&clmbtDst[0], dwDstSize);

	return true;
}

/// Decode 4
///
/// @param pclArc    Archive
/// @param pbtSrc    Compressed data
/// @param dwSrcSize Compressed data size
/// @param lWidth    Width
/// @param lHeight   Height
/// @param wBpp      Number of bits
///
bool CPB2A::Decode4(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp)
{
	// Ensure output buffer
	DWORD dwDstSize = (lWidth * lHeight * (wBpp >> 3));
	YCMemory<BYTE> clmbtDst(dwDstSize);

	// Decompression
	Decomp4(&clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp);

	// Output
	CImage clImage;
	clImage.Init(pclArc, lWidth, lHeight, wBpp);
	clImage.WriteReverse(&clmbtDst[0], dwDstSize);

	return true;
}

/// Decode 5
///
/// @param pclArc    Archive
/// @param pbtSrc    Compressed data
/// @param dwSrcSize Compressed data size
/// @param lWidth    Width
/// @param lHeight   Height
/// @param wBpp      Number of bits
///
bool CPB2A::Decode5(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp)
{
	// Ensure base image buffer
	DWORD dwBaseSize = (lWidth * lHeight * 4);
	YCMemory<BYTE> clmbtBase(dwBaseSize);

	// Decompress base image
	Decomp5(&clmbtBase[0], dwBaseSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp, NULL, 0);

	// Output base image
	CImage clImage;
	clImage.Init(pclArc, lWidth, lHeight, 32);
	clImage.WriteReverse(&clmbtBase[0], dwBaseSize);
	clImage.Close();

	// Get number of image files
	DWORD dwImageFiles = *(DWORD*)&pbtSrc[8];
	if (dwImageFiles == 1)
	{
		// Only the base image exists
		return true;
	}

	// Ensure image difference buffer
	DWORD dwDstSize = dwBaseSize;
	YCMemory<BYTE> clmbtDst(dwDstSize);

	// Output image difference
	for (DWORD dwFrameNumber = 1; dwFrameNumber < dwImageFiles; dwFrameNumber++)
	{
		// Decompress image difference
		Decomp5(&clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp, &clmbtBase[0], dwFrameNumber);

		// Output image difference
		TCHAR szDiffName[256];
		_stprintf(szDiffName, _T("_%02d"), (dwFrameNumber - 1));
		clImage.Init(pclArc, lWidth, lHeight, 32, NULL, 0, szDiffName);
		clImage.WriteReverse(&clmbtDst[0], dwDstSize, FALSE);
		clImage.Close();
	}

	return true;
}

/// Decode 6
///
/// @param pclArc    Archive
/// @param pbtSrc    Compressed data
/// @param dwSrcSize Compressed data size
/// @param lWidth    Width
/// @param lHeight   Height
/// @param wBpp      Number of bits
///
bool CPB2A::Decode6(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp)
{
	// Ensure base image buffer
	DWORD dwBaseSize = (lWidth * lHeight * 4);
	YCMemory<BYTE> clmbtBase(dwBaseSize);

	// Decompress base image
	Decomp6(&clmbtBase[0], dwBaseSize, pbtSrc, dwSrcSize, lWidth, lHeight, 32, NULL, 0);

	// Output base image
	CImage clImage;
	clImage.Init(pclArc, lWidth, lHeight, 32);
	clImage.WriteReverse(&clmbtBase[0], dwBaseSize);
	clImage.Close();

	// Get number of image files
	DWORD dwImageFiles = *(DWORD*)&pbtSrc[8];
	if (dwImageFiles == 1)
	{
		// Only the base image exists
		return true;
	}

	// Ensure difference image buffer
	DWORD dwDstSize = dwBaseSize;
	YCMemory<BYTE> clmbtDst(dwDstSize);

	// Output difference image 
	for (DWORD dwFrameNumber = 1; dwFrameNumber < dwImageFiles; dwFrameNumber++)
	{
		// Decompress difference image
		Decomp6(&clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, 32, &clmbtBase[0], dwFrameNumber);

		// Output difference image
		TCHAR szDiffName[256];
		_stprintf(szDiffName, _T("_%02d"), (dwFrameNumber - 1));
		clImage.Init(pclArc, lWidth, lHeight, 32, NULL, 0, szDiffName);
		clImage.WriteReverse(&clmbtDst[0], dwDstSize, FALSE);
		clImage.Close();
	}

	return true;
}

/// Decompression 1
///
/// @param pbtDst        Destination
/// @param dwDstSize     Destination size
/// @param pbtSrc        Compressed data
/// @param dwSrcSize     Compressed data size
/// @param lWidth        Width
/// @param lHeight       Height
/// @param wBpp          Number of bits
///
bool CPB2A::Decomp1(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp)
{
	DWORD dwOffsetOfFlags = *(DWORD*)&pbtSrc[24];
	DWORD dwOffsetOfCompData = *(DWORD*)&pbtSrc[28];
	long  lWidthOfBlock = 8;
	long  lHeightOfBlock = 8;
	WORD  wByteCount = (wBpp >> 3);
	long  lLine = (lWidth * wByteCount);

	// Ensure LZSS decompression buffer
	DWORD dwTempSize = (lWidth * lHeight * wByteCount);
	DWORD dwTempPtr = 0;
	YCMemory<BYTE> clmbtTemp(dwTempSize);

	// LZSS Decompression
	const BYTE* pbtFlags = &pbtSrc[dwOffsetOfFlags];
	const BYTE* pbtCompData = &pbtSrc[dwOffsetOfCompData];
	DWORD       dwFlagsSize = (dwOffsetOfCompData - dwOffsetOfFlags);
	DWORD       dwCompDataSize = (dwSrcSize - dwOffsetOfCompData);

	DecompLZSS(&clmbtTemp[0], dwTempSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize);

	// Change color componentss
	long lBlockCountOfWidth = ((lWidth + (lWidthOfBlock - 1)) / lWidthOfBlock);
	long lBlockCountOfHeight = ((lHeight + (lHeightOfBlock - 1)) / lHeightOfBlock);

	for (WORD wColor = 0; wColor < wByteCount; wColor++)
	{
		BYTE* pbtWorkOfDst = &pbtDst[wColor];

		for (long lY = 0, lBlockY = 0; lBlockY < lBlockCountOfHeight; lY += lHeightOfBlock, lBlockY++)
		{
			BYTE* pbtWorkOfDst2 = pbtWorkOfDst;
			long  lHeightOfBlockMax = ((lY + lHeightOfBlock) > lHeight) ? (lHeight - lY) : lHeightOfBlock;

			// Process the block in one column
			for (long lX = 0, lBlockX = 0; lBlockX < lBlockCountOfWidth; lX += lWidthOfBlock, lBlockX++)
			{
				BYTE* pbtWorkOfDst3 = pbtWorkOfDst2;
				long  lWidthOfBlockMax = ((lX + lWidthOfBlock) > lWidth) ? (lWidth - lX) : lWidthOfBlock;

				// Process 1 block
				for (long i = 0; i < lHeightOfBlockMax; i++)
				{
					for (long j = 0; j < lWidthOfBlockMax; j++)
					{
						pbtWorkOfDst3[j * wByteCount] = clmbtTemp[dwTempPtr++];
					}

					pbtWorkOfDst3 += lLine;
				}

				// Refer to the next block
				pbtWorkOfDst2 += (lWidthOfBlock * wByteCount);
			}

			// Refers to the bottom block
			pbtWorkOfDst += (lLine * lHeightOfBlock);
		}
	}

	return true;
}

/// Decompression 2
///
/// @param pbtDst        Destination
/// @param dwDstSize     Destination size
/// @param pbtSrc        Compressed data
/// @param dwSrcSize     Compressed data size
/// @param lWidth        Width
/// @param lHeight       Height
/// @param wBpp          Number of bits
///
bool CPB2A::Decomp2(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp)
{
	DWORD dwOffsetOfFlags = *(DWORD*)&pbtSrc[24];
	DWORD dwOffsetOfCompData = *(DWORD*)&pbtSrc[28];
	long  lWidthOfBlock = 8;
	long  lHeightOfBlock = 8;
	WORD  wByteCount = (wBpp >> 3);
	long  lLine = (lWidth * wByteCount);

	// Ensure LZSS decompression buffer
	DWORD dwTempSize = (lWidth * lHeight);
	YCMemory<BYTE> clmbtTemp(dwTempSize);

	// Decompression
	for (WORD wColor = 0; wColor < wByteCount; wColor++)
	{
		const BYTE* pbtWorkOfSrc = &pbtSrc[dwOffsetOfFlags];
		DWORD dwWork = (wByteCount * 4);

		for (WORD i = 0; i < wColor; i++)
		{
			dwWork += ((DWORD*)pbtWorkOfSrc)[i];
		}

		pbtWorkOfSrc += dwWork;

		// Get LZSS size after decompression
		DWORD dwLZSSDecodeSize = *(DWORD*)&pbtWorkOfSrc[8];

		// Get pointer to data flag
		const BYTE* pbtFlags = pbtWorkOfSrc + *(DWORD*)&pbtWorkOfSrc[0] + *(DWORD*)&pbtWorkOfSrc[4] + 12;

		// Get pointer to compressed data 
		const BYTE* pbtCompData = &pbtSrc[dwOffsetOfCompData];

		dwWork = (wByteCount * 4);
		for (WORD i = 0; i < wColor; i++)
		{
			dwWork += ((DWORD*)pbtCompData)[i];
		}
		pbtCompData += dwWork;

		// Handle insufficient buffer space
		// Do not need to gurantee the size of dwLZSSDecodeSize anymore, this minimizes allocation
		if (dwTempSize < dwLZSSDecodeSize)
		{
			clmbtTemp.resize(dwLZSSDecodeSize);
		}

		// LZSS Decompression
		DWORD dwFlagsSize = 0xFFFFFFFF;	// Unknown Size
		DWORD dwCompDataSize = 0xFFFFFFFF;	// Unknown Size
		DecompLZSS(&clmbtTemp[0], dwLZSSDecodeSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize);

		// Decompress compressed blocks
		long  lBlockCountOfWidth = ((lWidth + (lWidthOfBlock - 1)) / lWidthOfBlock);
		long  lBlockCountOfHeight = ((lHeight + (lHeightOfBlock - 1)) / lHeightOfBlock);
		BYTE  btCode = 0x80;
		DWORD dwTempPtr = 0;
		DWORD dwFlagsPtr = 0;
		DWORD dwCompDataPtr = 0;

		pbtFlags = pbtWorkOfSrc + 12;
		pbtCompData = pbtWorkOfSrc + *(DWORD*)&pbtWorkOfSrc[0] + 12;

		BYTE* pbtWorkOfDst = &pbtDst[wColor];

		for (long lY = 0, lBlockY = 0; lBlockY < lBlockCountOfHeight; lY += lHeightOfBlock, lBlockY++)
		{
			BYTE* pbtWorkOfDst2 = pbtWorkOfDst;
			long  lHeightOfBlockMax = ((lY + lHeightOfBlock) > lHeight) ? (lHeight - lY) : lHeightOfBlock;

			for (long lX = 0, lBlockX = 0; lBlockX < lBlockCountOfWidth; lX += lWidthOfBlock, lBlockX++)
			{
				if (btCode == 0)
				{
					// Processed 8 blocks

					dwFlagsPtr++;
					btCode = 0x80;
				}

				// Process 1 block

				BYTE* pbtWorkOfDst3 = pbtWorkOfDst2;
				long  lWidthOfBlockMax = ((lX + lWidthOfBlock) > lWidth) ? (lWidth - lX) : lWidthOfBlock;

				// Compressed
				if (pbtFlags[dwFlagsPtr] & btCode)
				{
					for (long i = 0; i < lHeightOfBlockMax; i++)
					{
						for (long j = 0; j < lWidthOfBlockMax; j++)
						{
							pbtWorkOfDst3[j * wByteCount] = pbtCompData[dwCompDataPtr];
						}

						pbtWorkOfDst3 += lLine;
					}

					dwCompDataPtr++;
				}
				else // Not compressed
				{
					for (long i = 0; i < lHeightOfBlockMax; i++)
					{
						for (long j = 0; j < lWidthOfBlockMax; j++)
						{
							pbtWorkOfDst3[j * wByteCount] = clmbtTemp[dwTempPtr++];
						}

						pbtWorkOfDst3 += lLine;
					}
				}

				// Refer to next block
				pbtWorkOfDst2 += (lWidthOfBlock * wByteCount);
				btCode >>= 1;
			}

			pbtWorkOfDst += (lLine * lHeightOfBlock);
		}
	}

	return true;
}

/// Decompression 4
///
/// @param pbtDst        Destination
/// @param dwDstSize     Destination size
/// @param pbtSrc        Compressed data
/// @param dwSrcSize     Compressed data size
/// @param lWidth        Width
/// @param lHeight       Height
/// @param wBpp          Number of bits
///
bool CPB2A::Decomp4(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp)
{
	// Get alpha value
	const BYTE* pbtAlpha = pbtSrc + *(DWORD*) &pbtSrc[24];
	DWORD dwAlphaSize = *(DWORD*) &pbtSrc[28];

	// Decompress
	CJBP1 clJBP1;
	clJBP1.Decomp( pbtDst, &pbtSrc[32], wBpp, pbtAlpha, dwAlphaSize );

	return true;
}

/// Decompression 5
///
/// Remark: if pbtBast is null, then difference compositing is not performed
///
/// @param pbtDst        Destination
/// @param dwDstSize     Destination size
/// @param pbtSrc        Compressed data
/// @param dwSrcSize     Compressed data size
/// @param lWidth        Width
/// @param lHeight       Height
/// @param wBpp          Number of bits
/// @param pbtBase       Base image
/// @param dwFrameNumber Frame number
///
bool CPB2A::Decomp5(
	BYTE*       pbtDst,
	DWORD       dwDstSize,
	const BYTE* pbtSrc,
	DWORD       dwSrcSize,
	long        lWidth,
	long        lHeight,
	WORD        wBpp,
	const BYTE* pbtBase,
	DWORD       dwFrameNumber
	)
{
	// Decompression

	// Base image
	if (dwFrameNumber == 0)
	{
		// Buffer allocation for LZSS extraction
		DWORD dwTempSize = (lWidth * lHeight);
		YCMemory<BYTE> aclmbtTemp[4];

		// LZSS Decompression
		for (WORD i = 0; i < 4; i++)
		{
			// Ensure the buffer can hold the LZSS data for extraction
			aclmbtTemp[i].resize(dwTempSize);

			// LZSS Decompression
			const BYTE* pbtFlags = pbtSrc + 64 + *(DWORD*)&pbtSrc[i * 8 + 32];
			const BYTE* pbtCompData = pbtSrc + 64 + *(DWORD*)&pbtSrc[i * 8 + 36];
			DWORD       dwFlagsSize = 0xFFFFFFFF;    // Unknown
			DWORD       dwCompDataSize = 0xFFFFFFFF; // Unknown

			DecompLZSS(&aclmbtTemp[i][0], dwTempSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize);
		}

		for (DWORD i = 0; i < dwTempSize; i++)
		{
			pbtDst[i * 4 + 0] = aclmbtTemp[0][i];
			pbtDst[i * 4 + 1] = aclmbtTemp[1][i];
			pbtDst[i * 4 + 2] = aclmbtTemp[2][i];
			pbtDst[i * 4 + 3] = aclmbtTemp[3][i];
		}

		// Decryption
		for (DWORD i = 0; i < dwTempSize; i++)
		{
			BYTE btData1 = pbtDst[i * 4 + 2] ^ pbtDst[i * 4 + 3];
			BYTE btData2 = pbtDst[i * 4 + 1] ^ btData1;

			pbtDst[i * 4 + 2] = btData1;
			pbtDst[i * 4 + 1] = btData2;
			pbtDst[i * 4 + 0] ^= btData2;
		}
	}
	else // Difference image
	{
		const BYTE* pbtWorkOfSrc = pbtSrc + *(DWORD*)&pbtSrc[12];
		for (DWORD i = 1; i < dwFrameNumber; i++)
		{
			pbtWorkOfSrc += *(DWORD*)&pbtWorkOfSrc[0];
		}

		// Buffer allocation for LZSS extraction
		DWORD dwLZSSDecodeSize = *(DWORD*)&pbtWorkOfSrc[12];
		YCMemory<BYTE> clmbtTemp(dwLZSSDecodeSize);

		// LZSS Decompression
		const BYTE* pbtFlags = pbtWorkOfSrc + 16;
		const BYTE* pbtCompData = pbtWorkOfSrc + 16 + *(DWORD*)&pbtWorkOfSrc[4];
		DWORD       dwFlagsSize = 0xFFFFFFFF;    // Unknown
		DWORD       dwCompDataSize = 0xFFFFFFFF; // Unknown
		DecompLZSS(&clmbtTemp[0], dwLZSSDecodeSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize);

		// Copy base image
		if (pbtBase != nullptr)
		{
			memcpy(pbtDst, pbtBase, dwDstSize);
		}
		else
		{
			memset(pbtDst, 0xFF, dwDstSize);
		}

		// Decompression
		long  lLine = (lWidth * 4);
		long  lWidthOfBlock = 8;
		long  lHeightOfBlock = 8;
		long  lBlockCountOfWidth = ((lWidth + (lWidthOfBlock - 1)) / lWidthOfBlock);
		long  lBlockCountOfHeight = ((lHeight + (lHeightOfBlock - 1)) / lHeightOfBlock);
		DWORD dwFlagsPtr = 0;
		DWORD dwCompDataPtr = 0;
		BYTE  btCode = 0x80;

		pbtFlags = &clmbtTemp[8];
		pbtCompData = &clmbtTemp[*(DWORD*)&clmbtTemp[0] + 8];

		BYTE* pbtWorkOfDst = pbtDst;

		for (long lY = 0, lBlockY = 0; lBlockY < lBlockCountOfHeight; lY += lHeightOfBlock, lBlockY++)
		{
			// Process the block of one column
			BYTE* pbtWorkOfDst2 = pbtWorkOfDst;
			long  lBlockOfHeightMax = ((lY + lHeightOfBlock) > lHeight) ? (lHeight - lY) : lHeightOfBlock;

			for (long lX = 0, lBlockX = 0; lBlockX < lBlockCountOfWidth; lX += lWidthOfBlock, lBlockX++)
			{
				if (btCode == 0)
				{
					// Processed 8 blocks
					btCode = 0x80;
					dwFlagsPtr++;
				}

				// Process 1 block
				BYTE* pbtWorkOfDst3 = pbtWorkOfDst2;
				long  lBlockOfWidthMax = ((lX + lWidthOfBlock) > lWidth) ? (lWidth - lX) : lWidthOfBlock;

				if ((pbtFlags[dwFlagsPtr] & btCode) == 0)
				{
					// Difference

					for (int i = 0; i < lBlockOfHeightMax; i++)
					{
						for (int j = 0; j < lBlockOfWidthMax; j++)
						{
							memcpy(&pbtWorkOfDst3[j * 4], &pbtCompData[dwCompDataPtr + j * 3], 3);

							pbtWorkOfDst3[j * 4 + 3] = 0xFF;
						}

						dwCompDataPtr += 3;
						pbtWorkOfDst3 += lLine;
					}
				}

				// Refer to next block
				pbtWorkOfDst2 += (lWidthOfBlock * 4);
				btCode >>= 1;
			}

			pbtWorkOfDst += (lLine * lHeightOfBlock);
		}
	}

	return true;
}

/// Decompress 6
///
/// Remark: If pbtBase is NULL, then difference compositing is not performed
///
/// @param pbtDst        Destination
/// @param dwDstSize     Destination size
/// @param pbtSrc        Compressed data
/// @param dwSrcSize     Compressed data size
/// @param lWidth        Width
/// @param lHeight       Height
/// @param wBpp          Number of bits
/// @param pbtBase       Base image
/// @param dwFrameNumber Frame number
///
bool CPB2A::Decomp6(
	BYTE*       pbtDst,
	DWORD       dwDstSize,
	const BYTE* pbtSrc,
	DWORD       dwSrcSize,
	long        lWidth,
	long        lHeight,
	WORD        wBpp,
	const BYTE* pbtBase,
	DWORD       dwFrameNumber
	)
{
	// Decompression

	// Base image
	if (dwFrameNumber == 0)
	{
		const BYTE* pbtBaseOfSrc = pbtSrc + 36 + ((strlen((char*)pbtSrc + 36) + 4) & 0xFFFFFFFC);

		// Buffer allocation for LZSS extraction
		DWORD          dwTempSize = (lWidth * lHeight);
		YCMemory<BYTE> aclmbtTemp[4];

		// LZSS Decompression
		for (WORD i = 0; i < 4; i++)
		{
			// Buffer allocation for LZSS extraction
			aclmbtTemp[i].resize(dwTempSize);

			// LZSS Decompression
			const BYTE* pbtFlags = pbtBaseOfSrc + 32 + *(DWORD*)&pbtBaseOfSrc[i * 8];
			const BYTE* pbtCompData = pbtBaseOfSrc + 32 + *(DWORD*)&pbtBaseOfSrc[i * 8 + 4];
			DWORD       dwFlagsSize = 0xFFFFFFFF;    // Unknown
			DWORD       dwCompDataSize = 0xFFFFFFFF; // Unknown

			DecompLZSS(&aclmbtTemp[i][0], dwTempSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize);
		}

		for (DWORD i = 0; i < dwTempSize; i++)
		{
			pbtDst[i * 4 + 0] = aclmbtTemp[0][i];
			pbtDst[i * 4 + 1] = aclmbtTemp[1][i];
			pbtDst[i * 4 + 2] = aclmbtTemp[2][i];
			pbtDst[i * 4 + 3] = aclmbtTemp[3][i];
		}

		// Decryption

		for (DWORD i = 0; i < dwTempSize; i++)
		{
			BYTE btData1 = pbtDst[i * 4 + 2] ^ pbtDst[i * 4 + 3];
			BYTE btData2 = pbtDst[i * 4 + 1] ^ btData1;

			pbtDst[i * 4 + 2] = btData1;
			pbtDst[i * 4 + 1] = btData2;
			pbtDst[i * 4 + 0] ^= btData2;
		}
	}
	else // Difference image
	{
		const BYTE* pbtSrcOfDiff = pbtSrc + *(DWORD*)&pbtSrc[32] + 32;
		for (DWORD i = 1; i < dwFrameNumber; i++)
		{
			pbtSrcOfDiff += *(DWORD*)&pbtSrcOfDiff[0];
		}

		// Get difference image file name
		char   szDiffName[256];
		size_t uNameLen = ((strlen((char*)pbtSrcOfDiff + 4) + 4) & 0xFFFFFFFC) + 4;
		strcpy(szDiffName, (char*)pbtSrcOfDiff + 4);

		// Buffer allocation for LZSS extraction
		DWORD          dwLZSSDecodeSize = *(DWORD*)&pbtSrcOfDiff[uNameLen + 8];
		YCMemory<BYTE> clmbtTemp(dwLZSSDecodeSize);

		// LZSS Decompression
		const BYTE* pbtFlags = pbtSrcOfDiff + 16 + uNameLen;
		const BYTE* pbtCompData = pbtSrcOfDiff + 16 + uNameLen + *(DWORD*)&pbtSrcOfDiff[uNameLen];
		DWORD       dwFlagsSize = 0xFFFFFFFF;    // Unknown
		DWORD       dwCompDataSize = 0xFFFFFFFF; // Unknown
		DecompLZSS(&clmbtTemp[0], dwLZSSDecodeSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize);

		// Copy base image
		if (pbtBase != nullptr)
		{
			memcpy(pbtDst, pbtBase, dwDstSize);
		}
		else
		{
			memset(pbtDst, 0xFF, dwDstSize);
		}

		// Decompression
		WORD  wByteCount = (wBpp >> 3);
		long  lLine = (lWidth * 4);
		long  lWidthOfBlock = 8;
		long  lHeightOfBlock = 8;
		long  lBlockCountOfWidth = ((lWidth + (lWidthOfBlock - 1)) / lWidthOfBlock);
		long  lBlockCountOfHeight = ((lHeight + (lHeightOfBlock - 1)) / lHeightOfBlock);
		DWORD dwFlagsPtr = 0;
		DWORD dwCompDataPtr = 0;
		BYTE  btCode = 0x80;

		pbtFlags = &clmbtTemp[8];
		pbtCompData = &clmbtTemp[*(DWORD*)&clmbtTemp[0] + 8];

		BYTE* pbtWorkOfDst = pbtDst;

		for (long lY = 0, lBlockY = 0; lBlockY < lBlockCountOfHeight; lY += lHeightOfBlock, lBlockY++)
		{
			BYTE* pbtWorkOfDst2 = pbtWorkOfDst;
			long  lBlockOfHeightMax = ((lY + lHeightOfBlock) > lHeight) ? (lHeight - lY) : lHeightOfBlock;

			// Process 1 column
			for (long lX = 0, lBlockX = 0; lBlockX < lBlockCountOfWidth; lX += lWidthOfBlock, lBlockX++)
			{
				if (btCode == 0)
				{
					// 8 blocks processed
					btCode = 0x80;
					dwFlagsPtr++;
				}

				// Process 1 block
				BYTE* pbtWorkOfDst3 = pbtWorkOfDst2;
				long  lBlockOfWidthMax = ((lX + lWidthOfBlock) > lWidth) ? (lWidth - lX) : lWidthOfBlock;

				if ((pbtFlags[dwFlagsPtr] & btCode) == 0)
				{
					// Difference
					for (long i = 0; i < lBlockOfHeightMax; i++)
					{
						for (long j = 0; j < lBlockOfWidthMax; j++)
						{
							memcpy(&pbtWorkOfDst3[j * 4], &pbtCompData[dwCompDataPtr + j * 4], 4);
						}

						dwCompDataPtr += (lBlockOfWidthMax * 4);
						pbtWorkOfDst3 += lLine;
					}
				}

				// Refer to the next block
				pbtWorkOfDst2 += (lWidthOfBlock * 4);
				btCode >>= 1;
			}

			pbtWorkOfDst += (lLine * lHeightOfBlock);
		}
	}

	return true;
}
