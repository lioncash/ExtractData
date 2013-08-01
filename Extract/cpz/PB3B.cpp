
#include	"stdafx.h"
#include	"../../Image.h"
#include	"JBP1.h"
#include	"PB3B.h"

//////////////////////////////////////////////////////////////////////////////////////////
//  Decoding
//
//  Remarks: pvSrc is decoded first

BOOL	CPB3B::Decode(
	CArcFile*			pclArc,							// Archive
	void*				pvSrc,							// Compressed data
	DWORD				dwSrcSize,						// Compressed data size
	PB3B_DECRYPT		pfnDecrypt						// Decoding function
	)
{
	BYTE*				pbtSrc = (BYTE*) pvSrc;

	// Decryption

	Decrypt( pbtSrc, dwSrcSize );

	// Get image information

	WORD				wType = *(WORD*) &pbtSrc[28];
	long				lWidth = *(WORD*) &pbtSrc[30];
	long				lHeight = *(WORD*) &pbtSrc[32];
	WORD				wBpp = *(WORD*) &pbtSrc[34];

	switch( wType )
	{
		case	1:
			Decode1( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
			break;

		case	2:
		case	3:
			Decode3( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
			break;

		case	4:
			Decode4( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
			break;

		case	5:
			Decode5( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
			break;

		case	6:
			Decode6( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp, pfnDecrypt );
			break;

		default:
			// Unknown

			pclArc->OpenFile();
			pclArc->WriteFile( pbtSrc, dwSrcSize );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decryption

void	CPB3B::Decrypt(
	BYTE*				pbtTarget,						// Decoded data
	DWORD				dwSize							// Decoded data size
	)
{
	for( int i = 8 ; i < 52 ; i += 2 )
	{
		*(WORD*) &pbtTarget[i] ^= *(WORD*) &pbtTarget[dwSize - 3];
	}

	for( int i = 8, j = 0 ; i < 52 ; i++, j++ )
	{
		pbtTarget[i] -= pbtTarget[dwSize - 47 + j];
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decode 1

BOOL	CPB3B::Decode1(
	CArcFile*			pclArc,							// Archive
	const BYTE*			pbtSrc,							// Compressed data
	DWORD				dwSrcSize,						// Compressed data size
	long				lWidth,							// Width
	long				lHeight,						// Height
	WORD				wBpp							// Number of bits per pixel
	)
{
	// Ensure output buffer

	DWORD				dwDstSize = (lWidth * lHeight * (wBpp >> 3));

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// Decompress

	Decomp1( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );

	// Output

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmbtDst[0], dwDstSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decode 3

BOOL	CPB3B::Decode3(
	CArcFile*			pclArc,							// Archive
	const BYTE*			pbtSrc,							// Compressed data
	DWORD				dwSrcSize,						// Compressed data size
	long				lWidth,							// Width
	long				lHeight,						// Height
	WORD				wBpp							// Number of bits per pixel
	)
{
	// Ensure output buffer

	DWORD				dwDstSize = (lWidth * lHeight * (wBpp >> 3));

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// Decompress

	Decomp3( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );

	// Output

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmbtDst[0], dwDstSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decode 4

BOOL	CPB3B::Decode4(
	CArcFile*			pclArc,							// Archive
	const BYTE*			pbtSrc,							// Compressed data
	DWORD				dwSrcSize,						// Compressed data size
	long				lWidth,							// Width
	long				lHeight,						// Height
	WORD				wBpp							// Number of bits per pixel
	)
{
	pclArc->OpenFile();
	pclArc->WriteFile( pbtSrc, dwSrcSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decode 5

BOOL	CPB3B::Decode5(
	CArcFile*			pclArc,							// Archive
	const BYTE*			pbtSrc,							// Compressed data
	DWORD				dwSrcSize,						// Compressed data size
	long				lWidth,							// Width
	long				lHeight,						// Height
	WORD				wBpp							// Number of bits per pixel
	)
{
	// Ensure base image buffer

	DWORD				dwBaseSize = (lWidth * lHeight * (wBpp >> 3));

	YCMemory<BYTE>		clmbtBase( dwBaseSize );

	// Decompress base image

	Decomp5( &clmbtBase[0], dwBaseSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );

	// Output base image

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmbtBase[0], dwBaseSize );
	clImage.Close();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decode 6

BOOL	CPB3B::Decode6(
	CArcFile*			pclArc,							// Archive
	const BYTE*			pbtSrc,							// Compressed data
	DWORD				dwSrcSize,						// Compressed data size
	long				lWidth,							// Width
	long				lHeight,						// Height
	WORD				wBpp,							// Number of bits per pixel
	PB3B_DECRYPT		pfnDecrypt						// Decryption function
	)
{
	// Ensure output buffer

	DWORD				dwDstSize = (lWidth * lHeight * (wBpp >> 3));

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// Get base image file name

	BYTE				abtBaseFileName[64];

	memcpy( abtBaseFileName, pbtSrc + 52, 32 );

	// Decrypt base image filename

	const static DWORD	adwCrypt[] =
	{
		0x9CF375A6, 0xA37869C5, 0x794FA53E, 0xC73AFE59
	};

	for( int i = 0 ; i < 2 ; i++ )
	{
		for( int j = 0 ; j < 4 ; j++ )
		{
			((DWORD*) abtBaseFileName)[i * 4 + j] ^= adwCrypt[j];
		}
	}

	abtBaseFileName[32] = '\0';

	YCString			clsBaseFileName = (char*) abtBaseFileName;

	clsBaseFileName += _T(".pb3");

	// プリミティブリンクのベース画像のファイル名が一部変になっているので修正

	if( clsBaseFileName == _T("ast-si0a_100.pb3") )
	{
		clsBaseFileName = _T("st-si0a_100a.pb3");
	}

	// Store base image

	const SFileInfo*		pstBaseFileInfo = pclArc->GetFileInfo( clsBaseFileName, TRUE );

	if( pstBaseFileInfo != NULL )
	{
		// Base image exists

		DWORD				dwBaseSrcSize = pstBaseFileInfo->sizeCmp;

		YCMemory<BYTE>		clmbtBaseSrc( dwBaseSrcSize );

		// Read base image

		pclArc->SeekHed( pstBaseFileInfo->start );
		pclArc->Read( &clmbtBaseSrc[0], dwBaseSrcSize );

		// Decrypt base image

		pfnDecrypt( &clmbtBaseSrc[0], dwBaseSrcSize, pclArc, *pstBaseFileInfo );
		Decrypt( &clmbtBaseSrc[0], dwBaseSrcSize );

		// Decompress base image

		Decomp5( &clmbtDst[0], dwDstSize, &clmbtBaseSrc[0], dwBaseSrcSize, lWidth, lHeight, wBpp );
	}
	else
	{
		// Base image does not exist

		ZeroMemory( &clmbtDst[0], dwDstSize );
	}

	// Decompress difference image

	Decomp6( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );

	// Output

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmbtDst[0], dwDstSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decompression 1

BOOL	CPB3B::Decomp1(
	BYTE*				pbtDst,							// Destination
	DWORD				dwDstSize,						// Destination size
	const BYTE*			pbtSrc,							// Compressed data
	DWORD				dwSrcSize,						// Compressed data size
	long				lWidth,							// Width
	long				lHeight,						// Height
	WORD				wBpp							// Number of bits per pixel
	)
{
	DWORD				dwOffsetOfFlags = *(DWORD*) &pbtSrc[44];
	DWORD				dwOffsetOfCompData = *(DWORD*) &pbtSrc[48];
	long				lWidthOfBlock = *(DWORD*) &pbtSrc[24];
	long				lHeightOfBlock = *(DWORD*) &pbtSrc[24];
	WORD				wByteCount = (wBpp >> 3);
	long				lLine = (lWidth * wByteCount);

	// Ensure LZSS decompression buffer

	DWORD				dwTempSize = (lWidth * lHeight);

	YCMemory<BYTE>		clmbtTemp( dwTempSize );

	// Decompress

	for( WORD wColor = 0 ; wColor < wByteCount ; wColor++ )
	{
		DWORD				dwWork;

		// 

		const BYTE*			pbtWorkOfSrc = &pbtSrc[dwOffsetOfFlags];

		dwWork = (wByteCount * 4);

		for( WORD i = 0 ; i < wColor ; i++ )
		{
			dwWork += ((DWORD*) pbtWorkOfSrc)[i];
		}

		pbtWorkOfSrc += dwWork;

		// Get the LZSS size after decompression

		DWORD				dwLZSSDecodeSize = *(DWORD*) &pbtWorkOfSrc[8];

		// Get pointer to flag data

		const BYTE*			pbtFlags = pbtWorkOfSrc + *(DWORD*) &pbtWorkOfSrc[0] + *(DWORD*) &pbtWorkOfSrc[4] + 12;

		// Get pointer to decompressed data

		const BYTE*			pbtCompData = &pbtSrc[dwOffsetOfCompData];

		dwWork = (wByteCount * 4);

		for( WORD i = 0 ; i < wColor ; i++ )
		{
			dwWork += ((DWORD*) pbtCompData)[i];
		}

		pbtCompData += dwWork;

		// バッファ不足に対処
		// dwLZSSDecodeSizeで毎回バッファ確保すれば対処する必要ないけど、メモリ確保／破棄は最小限に抑えたい

		if( dwTempSize < dwLZSSDecodeSize )
		{
			clmbtTemp.resize( dwLZSSDecodeSize );
		}

		// LZSS Decompression

		DWORD				dwFlagsSize = 0xFFFFFFFF;	// Unknown size
		DWORD				dwCompDataSize = 0xFFFFFFFF;	// Unknown size

		DecompLZSS( &clmbtTemp[0], dwLZSSDecodeSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize );

		// Decompress compressed blocks

		BYTE*				pbtWorkOfDst = &pbtDst[wColor];
		long				lBlockCountOfWidth = ((lWidth + (lWidthOfBlock - 1)) / lWidthOfBlock);
		long				lBlockCountOfHeight = ((lHeight + (lHeightOfBlock - 1)) / lHeightOfBlock);
		BYTE				btCode = 0x80;
		DWORD				dwTempPtr = 0;
		DWORD				dwFlagsPtr = 0;
		DWORD				dwCompDataPtr = 0;

		pbtFlags = pbtWorkOfSrc + 12;
		pbtCompData = pbtWorkOfSrc + *(DWORD*) &pbtWorkOfSrc[0] + 12;

		for( long lY = 0, lBlockY = 0 ; lBlockY < lBlockCountOfHeight ; lY += lHeightOfBlock, lBlockY++ )
		{
			BYTE*				pbtWorkOfDst2 = pbtWorkOfDst;
			long				lHeightOfBlockMax = ((lY + lHeightOfBlock) > lHeight) ? (lHeight - lY) : lHeightOfBlock;

			for( long lX = 0, lBlockX = 0 ; lBlockX < lBlockCountOfWidth ; lX += lWidthOfBlock, lBlockX++ )
			{
				if( btCode == 0 )
				{
					// 8-block processing

					dwFlagsPtr++;
					btCode = 0x80;
				}

				// Processing 1 block

				BYTE*				pbtWorkOfDst3 = pbtWorkOfDst2;
				long				lWidthOfBlockMax = ((lX + lWidthOfBlock) > lWidth) ? (lWidth - lX) : lWidthOfBlock;

				if( pbtFlags[dwFlagsPtr] & btCode )
				{
					// Is compressed

					for( long i = 0 ; i < lHeightOfBlockMax ; i++ )
					{
						for( long j = 0 ; j < lWidthOfBlockMax ; j++ )
						{
							pbtWorkOfDst3[j * wByteCount] = pbtCompData[dwCompDataPtr];
						}

						pbtWorkOfDst3 += lLine;
					}

					dwCompDataPtr++;
				}
				else
				{
					// Is not compressed

					for( long i = 0 ; i < lHeightOfBlockMax ; i++ )
					{
						for( long j = 0 ; j < lWidthOfBlockMax ; j++ )
						{
							pbtWorkOfDst3[j * wByteCount] = clmbtTemp[dwTempPtr++];
						}

						pbtWorkOfDst3 += lLine;
					}
				}

				// Point to the next block

				pbtWorkOfDst2 += (lWidthOfBlock * wByteCount);
				btCode >>= 1;
			}

			pbtWorkOfDst += (lLine * lHeightOfBlock);
		}
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decompression 3

BOOL	CPB3B::Decomp3(
	BYTE*				pbtDst,							// Destination
	DWORD				dwDstSize,						// Destination size
	const BYTE*			pbtSrc,							// Compressed data
	DWORD				dwSrcSize,						// Compressed data size
	long				lWidth,							// Width
	long				lHeight,						// Height
	WORD				wBpp							// Number of bits per pixel
	)
{
	// Get alpha value

	const BYTE*			pbtAlpha = pbtSrc + *(DWORD*) &pbtSrc[44];
	DWORD				dwAlphaSize = *(DWORD*) &pbtSrc[48];

	// Decompress

	CJBP1				clJBP1;

	clJBP1.Decomp( pbtDst, &pbtSrc[52], wBpp, pbtAlpha, dwAlphaSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decompression 4

BOOL	CPB3B::Decomp4(
	BYTE*				pbtDst,							// Destination
	DWORD				dwDstSize,						// Destination size
	const BYTE*			pbtSrc,							// Compressed data
	DWORD				dwSrcSize,						// Compressed data size
	long				lWidth,							// Width
	long				lHeight,						// Height
	WORD				wBpp							// Number of bits per pixel
	)
{
	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decompression 5

BOOL	CPB3B::Decomp5(
	BYTE*				pbtDst,							// Destination
	DWORD				dwDstSize,						// Destination size
	const BYTE*			pbtSrc,							// Compressed data
	DWORD				dwSrcSize,						// Compressed data size
	long				lWidth,							// Width
	long				lHeight,						// Height
	WORD				wBpp							// Number of bits per pixel
	)
{
	// LZSS decompression buffer

	DWORD				dwTempSize = (lWidth * lHeight);

	YCMemory<BYTE>		aclmbtTemp[4];

	// Decompress base image

	for( int i = 0 ; i < 4 ; i++ )
	{
		// Ensure LZSS decompression buffer

		aclmbtTemp[i].resize( dwTempSize );

		// Get pointer to flag data

		const BYTE*			pbtFlags = pbtSrc + 84 + *(DWORD*) &pbtSrc[i * 8 + 52];

		// Get pointer to compressed data

		const BYTE*			pbtCompData = pbtSrc + 84 + *(DWORD*) &pbtSrc[i * 8 + 56];

		// LZSS Decompression

		DWORD				dwFlagsSize = 0xFFFFFFFF;	// Unknown size
		DWORD				dwCompDataSize = 0xFFFFFFFF;	// Unknown size

		DecompLZSS( &aclmbtTemp[i][0], dwTempSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize );
	}

	// Store

	pbtDst[0] = aclmbtTemp[0][0];
	pbtDst[1] = aclmbtTemp[1][0];
	pbtDst[2] = aclmbtTemp[2][0];
	pbtDst[3] = aclmbtTemp[3][0];

	for( DWORD i = 1 ; i < dwTempSize ; i++ )
	{
		pbtDst[i * 4 + 0] = aclmbtTemp[0][i] + pbtDst[i * 4 - 4];
		pbtDst[i * 4 + 1] = aclmbtTemp[1][i] + pbtDst[i * 4 - 3];
		pbtDst[i * 4 + 2] = aclmbtTemp[2][i] + pbtDst[i * 4 - 2];
		pbtDst[i * 4 + 3] = aclmbtTemp[3][i] + pbtDst[i * 4 - 1];
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decompress 6
//
//  Remarks: Assumes that the base image is stored in pbtDst

BOOL	CPB3B::Decomp6(
	BYTE*				pbtDst,							// Destination
	DWORD				dwDstSize,						// Destination size
	const BYTE*			pbtSrc,							// Compressed data
	DWORD				dwSrcSize,						// Compressed data size
	long				lWidth,							// Width
	long				lHeight,						// Height
	WORD				wBpp							// Number of bits per pixel
	)
{
	// Ensure LZSS decompression buffer

	DWORD				dwLZSSDecodeSize = *(DWORD*) &pbtSrc[24];

	YCMemory<BYTE>		clmbtTemp( dwLZSSDecodeSize );

	// LZSS Decompression

	const BYTE*			pbtFlags = pbtSrc + 84;
	const BYTE*			pbtCompData = pbtSrc + 84 + *(DWORD*) &pbtSrc[44];
	DWORD				dwFlagsSize = 0xFFFFFFFF;	// Unknown size
	DWORD				dwCompDataSize = 0xFFFFFFFF;	// Unknown size

	DecompLZSS( &clmbtTemp[0], dwLZSSDecodeSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize );

	// Decompression

	WORD				wByteCount = (wBpp >> 3);
	long				lLine = (lWidth * 4);
	long				lWidthOfBlock = 8;
	long				lHeightOfBlock = 8;
	long				lBlockCountOfWidth = ((lWidth + (lWidthOfBlock - 1)) / lWidthOfBlock);
	long				lBlockCountOfHeight = ((lHeight + (lHeightOfBlock - 1)) / lHeightOfBlock);
	DWORD				dwFlagsPtr = 0;
	DWORD				dwCompDataPtr = 0;
	BYTE				btCode = 0x80;

	pbtFlags = &clmbtTemp[8];
	pbtCompData = &clmbtTemp[*(DWORD*) &clmbtTemp[0] + 8];

	BYTE*				pbtWorkOfDst = pbtDst;

	for( long lY = 0, lBlockY = 0 ; lBlockY < lBlockCountOfHeight ; lY += lHeightOfBlock, lBlockY++ )
	{
		BYTE*				pbtWorkOfDst2 = pbtWorkOfDst;
		long				lBlockOfHeightMax = ((lY + lHeightOfBlock) > lHeight) ? (lHeight - lY) : lHeightOfBlock;

		for( long lX = 0, lBlockX = 0 ; lBlockX < lBlockCountOfWidth ; lX += lWidthOfBlock, lBlockX++ )
		{
			if( btCode == 0 )
			{
				// Processing 8 blocks

				btCode = 0x80;
				dwFlagsPtr++;
			}

			// Process 1 block

			BYTE*				pbtWorkOfDst3 = pbtWorkOfDst2;
			long				lBlockOfWidthMax = ((lX + lWidthOfBlock) > lWidth) ? (lWidth - lX) : lWidthOfBlock;

			if( (pbtFlags[dwFlagsPtr] & btCode) == 0 )
			{
				// Difference

				for( long i = 0 ; i < lBlockOfHeightMax ; i++ )
				{
					for( long j = 0 ; j < lBlockOfWidthMax ; j++ )
					{
						memcpy( &pbtWorkOfDst3[j * 4], &pbtCompData[dwCompDataPtr + j * 4], 4 );
					}

					dwCompDataPtr += (lBlockOfWidthMax * 4);
					pbtWorkOfDst3 += lLine;
				}
			}

			// Point to the next block

			pbtWorkOfDst2 += (lWidthOfBlock * 4);
			btCode >>= 1;
		}

		pbtWorkOfDst += (lLine * lHeightOfBlock);
	}

	return	TRUE;
}
