#include "stdafx.h"
#include "../Image.h"
#include "LZSS.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Decode

BOOL CLZSS::Decode(
	CArcFile*			pclArc							// Archive
	)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->format != _T("LZ") )
	{
		return FALSE;
	}

	return Decomp( pclArc, 4096, 4078, 3 );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Extract the file

BOOL CLZSS::Decomp(
	CArcFile*			pclArc,							// Archive
	DWORD				dwDicSize,						// Dictionary Size
	DWORD				dwDicPtr,						// Initial dictionary position reference (Dictionary pointer)
	DWORD				dwLengthOffset					// Length offset
	)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	// Read
	DWORD          dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmSrc( dwSrcSize );
	pclArc->Read( &clmSrc[0], dwSrcSize );

	// Buffer allocation for extraction
	DWORD          dwDstSize = pstFileInfo->sizeOrg;
	YCMemory<BYTE> clmDst( dwDstSize );

	// Decompression
	Decomp( &clmDst[0], dwDstSize, &clmSrc[0], dwSrcSize, dwDicSize, dwDicPtr, dwLengthOffset );

	// Bitmap
	if( lstrcmp( PathFindExtension( pstFileInfo->name ), _T(".bmp") ) == 0 )
	{
		CImage clImage;
		clImage.Init( pclArc, &clmDst[0] );
		clImage.Write( dwDstSize );
		clImage.Close();
	}
	else // Other
	{
		pclArc->OpenFile();
		pclArc->WriteFile( &clmDst[0], dwDstSize );
		pclArc->CloseFile();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Extract from memory

BOOL CLZSS::Decomp(
	void*				pvDst,							// Destination
	DWORD				dwDstSize,						// Destination Size
	const void*			pvSrc,							// Compressed Data
	DWORD				dwSrcSize,						// Compressed Data Size
	DWORD				dwDicSize,						// Dictionary Size
	DWORD				dwDicPtr,						// Initial reference to dictionary position (Dictionary pointer)
	DWORD				dwLengthOffset					// Length offset
	)
{
	BYTE*       pbtDst = (BYTE*) pvDst;
	const BYTE* pbtSrc = (const BYTE*) pvSrc;

	// Allocate dictionary buffer
	YCMemory<BYTE> clmbtDic( dwDicSize );
	ZeroMemory( &clmbtDic[0], dwDicSize );

	// Decompression
	DWORD dwSrcPtr = 0;
	DWORD dwDstPtr = 0;
	BYTE  btFlags;
	DWORD dwBitCount = 0;

	while( (dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) )
	{
		if( dwBitCount == 0 )
		{
			// Finished reading 8-bits

			btFlags = pbtSrc[dwSrcPtr++];
			dwBitCount = 8;
		}

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
			DWORD dwLength = ((btHigh & 0x0F) + dwLengthOffset);

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
