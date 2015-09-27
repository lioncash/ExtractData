#include "stdafx.h"
#include "../../ExtractBase.h"
#include "PB.h"

/// Decompress LZSS
///
/// @param pvDst      Destination
/// @param dwDstSize  Destination size
/// @param pvFlags    Flag data
/// @param dwFlagSize Flag data size
/// @param pvSrc      Compressed data
/// @param dwSrcSize  Compressed data size
///
BOOL CPB::DecompLZSS(void* pvDst, DWORD dwDstSize, const void* pvFlags, DWORD dwFlagsSize, const void* pvSrc, DWORD dwSrcSize)
{
	// Initialize Dictionary
	DWORD dwDicSize = 2048;
	YCMemory<BYTE> clmbtDic(dwDicSize);
	ZeroMemory(&clmbtDic[0], dwDicSize);
	DWORD dwDicPtr = 2014;

	// Decompress
	const BYTE* pbtSrc = (const BYTE*)pvSrc;
	const BYTE* pbtFlags = (const BYTE*)pvFlags;
	BYTE*       pbtDst = (BYTE*)pvDst;
	DWORD       dwSrcPtr = 0;
	DWORD       dwFlagsPtr = 0;
	DWORD       dwDstPtr = 0;
	BYTE        btCode = 0x80;

	while ((dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) && (dwFlagsPtr < dwFlagsSize))
	{
		if (btCode == 0)
		{
			dwFlagsPtr++;
			btCode = 0x80;
		}

		// Is Compressed
		if (pbtFlags[dwFlagsPtr] & btCode)
		{
			WORD  wWork = *(WORD*)&pbtSrc[dwSrcPtr];

			DWORD dwBack = (wWork >> 5);
			DWORD dwLength = (wWork & 0x1F) + 3;

			// Adjust so that the buffer is not exceeded
			if ((dwDstPtr + dwLength) > dwDstSize)
			{
				dwLength = (dwDstSize - dwDstPtr);
			}

			// Dictionary Reference
			for (DWORD i = 0; i < dwLength; i++)
			{
				pbtDst[dwDstPtr + i] = clmbtDic[dwDicPtr++] = clmbtDic[dwBack++];

				dwDicPtr &= (dwDicSize - 1);
				dwBack &= (dwDicSize - 1);
			}

			dwSrcPtr += 2;
			dwDstPtr += dwLength;
		}
		else // Not compressed
		{
			pbtDst[dwDstPtr++] = clmbtDic[dwDicPtr++] = pbtSrc[dwSrcPtr++];

			dwDicPtr &= (dwDicSize - 1);
		}

		btCode >>= 1;
	}

	return TRUE;
}
