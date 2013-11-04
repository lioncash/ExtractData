#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Image.h"
#include "Tga.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Decoding

BOOL CTga::Decode(
	CArcFile*			pclArc,							// Archive
	const void*			pvSrc,							// TGA Data
	DWORD				dwSrcSize,						// TGA Data Size
	const YCString&		rfclsFileLastName				// End of the filename
	)
{
	const BYTE*       pbtSrc = (const BYTE*) pvSrc;
	const STGAHeader* psttgahSrc = (STGAHeader*) pvSrc;

	pbtSrc += sizeof(STGAHeader);
	dwSrcSize -= sizeof(STGAHeader);

	// Decompression

	YCMemory<BYTE> clmbtSrc2;

	switch( psttgahSrc->btImageType )
	{
	case 9:
	case 10:
		// RLE Compression

		DWORD				dwSrcSize2 = ((psttgahSrc->wWidth * (psttgahSrc->btDepth >> 3) + 3) & 0xFFFFFFFC) * psttgahSrc->wHeight;

		clmbtSrc2.resize( dwSrcSize2 );

		DecompRLE( &clmbtSrc2[0], dwSrcSize2, pbtSrc, dwSrcSize, psttgahSrc->btDepth );

		pbtSrc = &clmbtSrc2[0];
		dwSrcSize = dwSrcSize2;

		break;
	}

	CImage clImage;

	if( psttgahSrc->btDepth == 0 )
	{
		clImage.Init( pclArc, psttgahSrc->wWidth, psttgahSrc->wHeight, 32, NULL, 0, rfclsFileLastName );
		clImage.WriteReverse( pbtSrc, dwSrcSize );
		clImage.Close();
	}
	else
	{
		clImage.Init( pclArc, psttgahSrc->wWidth, psttgahSrc->wHeight, psttgahSrc->btDepth, NULL, 0, rfclsFileLastName );
		clImage.Write( pbtSrc, dwSrcSize );
		clImage.Close();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decompression of compressed TGAs

BOOL CTga::Decomp(
	void*				pvDst,							// Storage location
	DWORD				dwDstSize,						// Storage location size
	const void*			pvSrc,							// Compressed TGA
	DWORD				dwSrcSize						// Compressed TGA Size
	)
{
	BYTE*             pbtDst = (BYTE*) pvDst;
	const BYTE*       pbtSrc = (const BYTE*) pvSrc;
	const STGAHeader* psttgahSrc = (STGAHeader*) pvSrc;

	pbtSrc += sizeof(STGAHeader);
	dwSrcSize -= sizeof(STGAHeader);

	// Decompression

	switch( psttgahSrc->btDepth )
	{
	case 9:
	case 10:
		// RLE

		DecompRLE( pbtDst, dwDstSize, pbtSrc, dwSrcSize, psttgahSrc->btDepth );
		break;

	default:
		// No Compression

		memcpy( pbtDst, pbtSrc, dwSrcSize );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// RLE Decompression

BOOL CTga::DecompRLE(
	void*				pvDst,							// Storage Location
	DWORD				dwDstSize,						// Storage Location Size
	const void*			pvSrc,							// Compressed Data
	DWORD				dwSrcSize,						// Compressed Data Size
	BYTE				wBpp							// Number of bits
	)
{
	const BYTE* pbtSrc = (const BYTE*) pvSrc;
	BYTE*       pbtDst = (BYTE*) pvDst;
	DWORD       dwSrcPtr = 0;
	DWORD       dwDstPtr = 0;
	WORD        wByteCount = (wBpp >> 3);

	while( (dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) )
	{
		DWORD dwLength = pbtSrc[dwSrcPtr++];

		if( dwLength & 0x80 )
		{
			dwLength = (dwLength & 0x7F) + 1;

			for( DWORD i = 0 ; i < dwLength ; i++ )
			{
				memcpy( &pbtDst[dwDstPtr], &pbtSrc[dwSrcPtr], wByteCount );

				dwDstPtr += wByteCount;
			}

			dwSrcPtr += wByteCount;
		}
		else
		{
			dwLength++;

			dwLength *= wByteCount;

			memcpy( &pbtDst[dwDstPtr], &pbtSrc[dwSrcPtr], dwLength );

			dwSrcPtr += dwLength;
			dwDstPtr += dwLength;
		}

		if( memcmp( &pbtSrc[dwSrcPtr + 8], "TRUEVISION-XFILE", 16 ) == 0 )
		{
			// End

			break;
		}
	}

	return TRUE;
}
