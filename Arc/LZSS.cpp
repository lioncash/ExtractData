#include "stdafx.h"
#include "../Image.h"
#include "LZSS.h"

/// Decode
///
/// @param pclArc Archive
///
BOOL CLZSS::Decode(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	if (pstFileInfo->format != _T("LZ"))
	{
		return FALSE;
	}

	return Decomp(pclArc, 4096, 4078, 3);
}

/// Extract the file
///
/// @param pclArc         Archive
/// @param dwDictSize     Dictionary size
/// @param dwDicPtr       Initial address to the dictionary position (dictionary pointer)
/// @param dwLengthOffset Length offset
///
BOOL CLZSS::Decomp(CArcFile* pclArc, DWORD dwDicSize, DWORD dwDicPtr, DWORD dwLengthOffset)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	// Read
	DWORD          dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmSrc(dwSrcSize);
	pclArc->Read(&clmSrc[0], dwSrcSize);

	// Buffer allocation for extraction
	DWORD          dwDstSize = pstFileInfo->sizeOrg;
	YCMemory<BYTE> clmDst(dwDstSize);

	// Decompression
	Decomp(&clmDst[0], dwDstSize, &clmSrc[0], dwSrcSize, dwDicSize, dwDicPtr, dwLengthOffset);

	// Bitmap
	if (lstrcmp(PathFindExtension(pstFileInfo->name), _T(".bmp")) == 0)
	{
		CImage clImage;
		clImage.Init(pclArc, &clmDst[0]);
		clImage.Write(dwDstSize);
		clImage.Close();
	}
	else // Other
	{
		pclArc->OpenFile();
		pclArc->WriteFile(&clmDst[0], dwDstSize);
		pclArc->CloseFile();
	}

	return TRUE;
}

/// Extract from memory
///
/// @param pvDst          Destination
/// @param dwDstSize      Destination size
/// @param pvSrc          Compressed data
/// @param dwSrcSize      Compressed data size
/// @param dwDictSize     Dictionary size
/// @param dwDicPtr       Initial address to the dictionary position (dictionary pointer)
/// @param dwLengthOffset Length offset
///
BOOL CLZSS::Decomp(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, DWORD dwDicSize, DWORD dwDicPtr, DWORD dwLengthOffset)
{
	BYTE*       pbtDst = static_cast<BYTE*>(pvDst);
	const BYTE* pbtSrc = static_cast<const BYTE*>(pvSrc);

	// Allocate dictionary buffer
	std::vector<BYTE> clmbtDic(dwDicSize);

	// Decompression
	DWORD dwSrcPtr = 0;
	DWORD dwDstPtr = 0;
	BYTE  btFlags = 0;
	DWORD dwBitCount = 0;

	while (dwSrcPtr < dwSrcSize && dwDstPtr < dwDstSize)
	{
		if (dwBitCount == 0)
		{
			// Finished reading 8-bits

			btFlags = pbtSrc[dwSrcPtr++];
			dwBitCount = 8;
		}

		if (btFlags & 1)
		{
			// Non-compressed data

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

			DWORD dwBack = (((btHigh & 0xF0) << 4) | btLow);
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


/*
		for( DWORD i = 0 ; (i < 8) && (dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) ; i++ )
		{
			if( btFlags & 1 )
			{
				// Non-compressed data

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

				DWORD dwBack = (((btHigh & 0xF0) << 4) | btLow);
				DWORD dwLength = ((btHigh & 0x0F) + dwOffset);

				if( (dwDstPtr + dwLength) > dwDstSize )
				{
					// Exceeds the output buffer

					dwLength = (dwDstSize - dwDstPtr);
				}

				for( DWORD j = 0 ; j < dwLength ; j++ )
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
		}*/
	}

	return TRUE;
}
