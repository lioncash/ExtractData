#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Image.h"
#include "Pajamas.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Mounting

BOOL CPajamas::Mount(
	CArcFile*			pclArc							// Archive
	)
{
	if( MountDat1( pclArc ) )
	{
		return TRUE;
	}

	if( MountDat2( pclArc ) )
	{
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// GAMEDAT PACK Mounting

BOOL CPajamas::MountDat1(
	CArcFile*			pclArc							// Archive
	)
{
	if( (pclArc->GetArcExten() != _T(".dat")) && (pclArc->GetArcExten() != _T(".pak")) )
	{
		return FALSE;
	}

	if( memcmp( pclArc->GetHed(), "GAMEDAT PACK", 12 ) != 0 )
	{
		return FALSE;
	}

	// Get file count
	DWORD dwFiles;
	pclArc->SeekHed( 12 );
	pclArc->Read( &dwFiles, 4 );

	// Get index size from file count
	DWORD dwIndexSize = dwFiles * 24;

	// Get index
	YCMemory<BYTE> clmbtIndex( dwIndexSize );
	DWORD          dwIndexPtr = 0;
	pclArc->Read( &clmbtIndex[0], dwIndexSize );

	// Get index of the file information
	DWORD dwFileNameIndexSize = dwFiles << 4;
	BYTE* pbtFileInfoIndex = &clmbtIndex[dwFileNameIndexSize];

	// Get offset (Required for correction when the starting address is zero-based)
	DWORD dwOffset = 16 + dwIndexSize;

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// Get filename
		TCHAR szFileName[16];
		memcpy( szFileName, &clmbtIndex[dwIndexPtr], 16 );

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.start = *(DWORD*) &pbtFileInfoIndex[0] + dwOffset;
		stFileInfo.sizeCmp = *(DWORD*) &pbtFileInfoIndex[4];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );

		dwIndexPtr += 16;
		pbtFileInfoIndex += 8;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// GAMEDAT PAC2 Mounting

BOOL CPajamas::MountDat2(
	CArcFile*			pclArc							// Archive
	)
{
	if( pclArc->GetArcExten() != _T(".dat") )
	{
		return FALSE;
	}

	if( memcmp( pclArc->GetHed(), "GAMEDAT PAC2", 12 ) != 0 )
	{
		return FALSE;
	}

	// Get file count
	DWORD dwFiles;
	pclArc->SeekHed( 12 );
	pclArc->Read( &dwFiles, 4 );

	// Get index size from file count
	DWORD dwIndexSize = dwFiles * 40;

	// Get index
	YCMemory<BYTE> clmbtIndex( dwIndexSize );
	DWORD         dwIndexPtr = 0;
	pclArc->Read( &clmbtIndex[0], dwIndexSize );

	// Get index of the file information
	DWORD dwFileNameIndexSize = dwFiles << 5;
	BYTE* pbtFileInfoIndex = &clmbtIndex[dwFileNameIndexSize];

	// Get offset (Required for correction when the starting address is zero-based)
	DWORD dwOffset = 16 + dwIndexSize;

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// Get filename
		TCHAR szFileName[32];
		memcpy( szFileName, &clmbtIndex[dwIndexPtr], 32 );

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.start = *(DWORD*) &pbtFileInfoIndex[0] + dwOffset;
		stFileInfo.sizeCmp = *(DWORD*) &pbtFileInfoIndex[4];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );

		dwIndexPtr += 32;
		pbtFileInfoIndex += 8;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decoding

BOOL CPajamas::Decode(
	CArcFile*			pclArc							// Archive
	)
{
	if( DecodeEPA( pclArc ) )
	{
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// EPA Decoding

BOOL CPajamas::DecodeEPA(
	CArcFile*			pclArc							// Archive
	)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	if( pstFileInfo->format != _T("EPA") )
	{
		return FALSE;
	}

	// Read header
	BYTE abtHeader[16];
	pclArc->Read( abtHeader, sizeof(abtHeader) );

	// Difference flag, Number of colors, width, height
	BYTE btDiffFlag = abtHeader[3];
	WORD wBpp = abtHeader[4];
	long lWidth = *(long*) &abtHeader[8];
	long lHeight = *(long*) &abtHeader[12];

	switch( wBpp )
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

	switch( btDiffFlag )
	{
	case 1: // Usual
		break;

	case 2: // Difference
		pclArc->SeekCur( 40 );
		break;

	default: // Unknown
		pclArc->SeekHed( pstFileInfo->start );
		return FALSE;
	}

	// Read palette
	BYTE abtPallet[768];
	if( wBpp == 8 )
	{
		pclArc->Read( abtPallet, sizeof(abtPallet) );
	}

	// Read EPA data
	DWORD dwSrcSize = pstFileInfo->sizeCmp - 16;
	if( wBpp == 8 )
	{
		dwSrcSize -= sizeof(abtPallet);
	}
	YCMemory<BYTE> clmbtSrc( dwSrcSize );
	pclArc->Read( &clmbtSrc[0], dwSrcSize );

	// Secure area to store the BMP data
	DWORD dwDstSize = lWidth * lHeight * (wBpp >> 3);
	YCMemory<BYTE> clmbtDst( dwDstSize );
	ZeroMemory( &clmbtDst[0], dwDstSize );

	// Decompress EPA
	DecompEPA( &clmbtDst[0], dwDstSize, &clmbtSrc[0], dwSrcSize, lWidth );

	// Output
	CImage clImage;
	clImage.Init( pclArc, lWidth, lHeight, wBpp, abtPallet, sizeof(abtPallet) );
	clImage.WriteCompoBGRAReverse( &clmbtDst[0], dwDstSize );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// EPA Decompression

BOOL CPajamas::DecompEPA(
	void*				pvDst,							// Destination
	DWORD				dwDstSize,						// Destination Size
	const void*			pvSrc,							// Input/Compressed data
	DWORD				dwSrcSize,						// Input/Compressed data size
	long				lWidth							// Width
	)
{
	const BYTE* pbtSrc = (const BYTE*) pvSrc;
	BYTE*       pbtDst = (BYTE*) pvDst;

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
	while( (dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) )
	{
		BYTE btCode = pbtSrc[dwSrcPtr++];
		DWORD dwLength = btCode & 0x07;

		if( btCode & 0xF0 )
		{
			if( btCode & 0x08 )
			{
				dwLength = (dwLength << 8) + pbtSrc[dwSrcPtr++];
			}

			if( dwLength != 0 )
			{
				btCode >>= 4;

				DWORD dwBack = dwDstPtr - adwOffsets[btCode];

				if( (dwDstPtr + dwLength) > dwDstSize )
				{
					// Exceeds output buffer
					dwLength = (dwDstSize - dwDstPtr);
				}

				for( DWORD i = 0 ; i < dwLength ; i++ )
				{
					pbtDst[dwDstPtr + i] = pbtDst[dwBack + i];
				}

				dwDstPtr += dwLength;
			}
		}
		else if( btCode != 0 )
		{
			memcpy( &pbtDst[dwDstPtr], &pbtSrc[dwSrcPtr], btCode );

			dwSrcPtr += btCode;
			dwDstPtr += btCode;
		}
	}

	return TRUE;
}
