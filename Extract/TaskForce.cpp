#include	"stdafx.h"
#include	"../Arc/LZSS.h"
#include	"../Image.h"
#include	"../Image/Tga.h"
#include	"TaskForce.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	Mounting

BOOL	CTaskForce::Mount(
	CArcFile*			pclArc							// Archive
	)
{
	if( MountDat( pclArc ) )
	{
		return	TRUE;
	}

	if( MountTlz( pclArc ) )
	{
		return	TRUE;
	}

	if( MountBma( pclArc ) )
	{
		return	TRUE;
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	dat mounting

BOOL	CTaskForce::MountDat(
	CArcFile*			pclArc							// Archive
	)
{
	if( pclArc->GetArcExten() != _T(".dat") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "tskforce", 8 ) != 0 )
	{
		return	FALSE;
	}

	pclArc->SeekHed( 8 );

	// Get file count

	DWORD				dwFiles;

	pclArc->Read( &dwFiles, 4 );

	// Get index

	YCMemory<SFileEntry>	clmIndex( dwFiles );

	pclArc->Read( &clmIndex[0], (sizeof(SFileEntry) * dwFiles) );

	// Get file information

	DWORD				dwIndexPtr = 0;

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		SFileInfo			stFileInfo;

		stFileInfo.name = clmIndex[i].szFileName;
		stFileInfo.sizeCmp = clmIndex[i].dwCompressedSize;
		stFileInfo.sizeOrg = clmIndex[i].dwOriginalSize;
		stFileInfo.start = clmIndex[i].dwOffset;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		if( stFileInfo.sizeCmp != stFileInfo.sizeOrg )
		{
			stFileInfo.format = _T("LZ");
		}

		pclArc->AddFileInfo( stFileInfo );

		dwIndexPtr += sizeof(SFileEntry);
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	tlz mounting

BOOL	CTaskForce::MountTlz(
	CArcFile*			pclArc							// Archive
	)
{
	if( (pclArc->GetArcExten() != _T(".tsk")) && (pclArc->GetArcExten() != _T(".tfz")) )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "tlz", 3 ) != 0 )
	{
		return	FALSE;
	}

	return	pclArc->Mount();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	bma mounting

BOOL	CTaskForce::MountBma(
	CArcFile*			pclArc							// Archive
	)
{
	if( pclArc->GetArcExten() != _T(".tsz") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "bma", 3 ) != 0 )
	{
		return	FALSE;
	}

	return	pclArc->Mount();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Decoding

BOOL	CTaskForce::Decode(
	CArcFile*			pclArc							// Archive
	)
{
	if( DecodeTlz( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodeBma( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodeTGA( pclArc ) )
	{
		return	TRUE;
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	tlz decoding

BOOL	CTaskForce::DecodeTlz(
	CArcFile*			pclArc							// Archive
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( (pstFileInfo->name.GetFileExt() != _T(".tsk")) && (pstFileInfo->name.GetFileExt() != _T(".tfz")) )
	{
		return	FALSE;
	}

	// Read header

	BYTE				abtHeader[24];

	pclArc->Read( abtHeader, sizeof(abtHeader) );

	if( memcmp( abtHeader, "tlz", 3 ) != 0 )
	{
		pclArc->SeekHed( pstFileInfo->start );
		return	FALSE;
	}

	// Get file information

	DWORD				dwDstSize = *(DWORD*) &abtHeader[16];
	DWORD				dwSrcSize = *(DWORD*) &abtHeader[20];

	// Read compressed data

	YCMemory<BYTE>		clmSrc( dwSrcSize );

	pclArc->Read( &clmSrc[0], dwSrcSize );

	// Buffer allocation for decompression

	YCMemory<BYTE>		clmDst( dwDstSize );

	// LZSS Decompression

	CLZSS				clLZSS;

	clLZSS.Decomp( &clmDst[0], dwDstSize, &clmSrc[0], dwSrcSize, 4096, 4078, 3 );

	// Output

	pclArc->OpenFile();
	pclArc->WriteFile( &clmDst[0], dwDstSize, dwSrcSize );
	pclArc->CloseFile();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	bma decoding

BOOL	CTaskForce::DecodeBma(
	CArcFile*			pclArc							// Archive
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->name.GetFileExt() != _T(".tsz") )
	{
		return	FALSE;
	}

	// Read header

	BYTE				abtHeader[24];

	pclArc->Read( abtHeader, sizeof(abtHeader) );

	if( memcmp( abtHeader, "bma", 3 ) != 0 )
	{
		pclArc->SeekHed( pstFileInfo->start );
		return	FALSE;
	}

	// Get file information

	long				lWidth = *(long*) &abtHeader[4];
	long				lHeight = *(long*) &abtHeader[8];
	DWORD				dwDstSize = *(DWORD*) &abtHeader[16];
	DWORD				dwSrcSize = *(DWORD*) &abtHeader[20];

	// Read compressed data

	YCMemory<BYTE>		clmSrc( dwSrcSize );

	pclArc->Read( &clmSrc[0], dwSrcSize );

	// Buffer allocation for decompression

	YCMemory<BYTE>		clmDst( dwDstSize );

	// LZSS Decoding

	CLZSS				clLZSS;

	clLZSS.Decomp( &clmDst[0], dwDstSize, &clmSrc[0], dwSrcSize, 4096, 4078, 3 );

	// Output

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, 32 );
	clImage.WriteReverse( &clmDst[0], dwDstSize );
	clImage.Close();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	TGA Decoding

BOOL	CTaskForce::DecodeTGA(
	CArcFile*			pclArc							// Archive
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->name.GetFileExt() != _T(".tga") )
	{
		return	FALSE;
	}

	// Read data

	DWORD				dwSrcSize = pstFileInfo->sizeCmp;

	YCMemory<BYTE>		clmSrc( dwSrcSize );

	pclArc->Read( &clmSrc[0], dwSrcSize );

	if( pstFileInfo->format == _T("LZ") )
	{
		// Is compressed

		DWORD				dwDstSize = pstFileInfo->sizeOrg;

		YCMemory<BYTE>		clmDst( dwDstSize );

		// LZSS Decompression

		CLZSS				clLZSS;

		clLZSS.Decomp( &clmDst[0], dwDstSize, &clmSrc[0], dwSrcSize, 4096, 4078, 3 );

		// Output

		CTga				clTGA;

		clTGA.Decode( pclArc, &clmDst[0], dwDstSize );
	}
	else
	{
		// Uncompressed

		CTga				clTGA;

		clTGA.Decode( pclArc, &clmSrc[0], dwSrcSize );
	}

	return	TRUE;
}
