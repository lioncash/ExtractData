
#include	"stdafx.h"
#include	"../Arc/LZSS.h"
#include	"../Image.h"
#include	"../Tga.h"
#include	"TaskForce.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CTaskForce::Mount(
	CArcFile*			pclArc							// アーカイブ
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
//	datのマウント

BOOL	CTaskForce::MountDat(
	CArcFile*			pclArc							// アーカイブ
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

	// ファイル数の取得

	DWORD				dwFiles;

	pclArc->Read( &dwFiles, 4 );

	// インデックスの取得

	YCMemory<SFileEntry>	clmIndex( dwFiles );

	pclArc->Read( &clmIndex[0], (sizeof(SFileEntry) * dwFiles) );

	// ファイル情報の取得

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
//	tlzのマウント

BOOL	CTaskForce::MountTlz(
	CArcFile*			pclArc							// アーカイブ
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
//	bmaのマウント

BOOL	CTaskForce::MountBma(
	CArcFile*			pclArc							// アーカイブ
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
//	デコード

BOOL	CTaskForce::Decode(
	CArcFile*			pclArc							// アーカイブ
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
//	tlzのデコード

BOOL	CTaskForce::DecodeTlz(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( (pstFileInfo->name.GetFileExt() != _T(".tsk")) && (pstFileInfo->name.GetFileExt() != _T(".tfz")) )
	{
		return	FALSE;
	}

	// ヘッダの読み込み

	BYTE				abtHeader[24];

	pclArc->Read( abtHeader, sizeof(abtHeader) );

	if( memcmp( abtHeader, "tlz", 3 ) != 0 )
	{
		pclArc->SeekHed( pstFileInfo->start );
		return	FALSE;
	}

	// ファイル情報の取得

	DWORD				dwDstSize = *(DWORD*) &abtHeader[16];
	DWORD				dwSrcSize = *(DWORD*) &abtHeader[20];

	// 圧縮データの読み込み

	YCMemory<BYTE>		clmSrc( dwSrcSize );

	pclArc->Read( &clmSrc[0], dwSrcSize );

	// 解凍後のバッファの確保

	YCMemory<BYTE>		clmDst( dwDstSize );

	// LZSSの解凍

	CLZSS				clLZSS;

	clLZSS.Decomp( &clmDst[0], dwDstSize, &clmSrc[0], dwSrcSize, 4096, 4078, 3 );

	// 出力

	pclArc->OpenFile();
	pclArc->WriteFile( &clmDst[0], dwDstSize, dwSrcSize );
	pclArc->CloseFile();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	bmaのデコード

BOOL	CTaskForce::DecodeBma(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->name.GetFileExt() != _T(".tsz") )
	{
		return	FALSE;
	}

	// ヘッダの読み込み

	BYTE				abtHeader[24];

	pclArc->Read( abtHeader, sizeof(abtHeader) );

	if( memcmp( abtHeader, "bma", 3 ) != 0 )
	{
		pclArc->SeekHed( pstFileInfo->start );
		return	FALSE;
	}

	// ファイル情報の取得

	long				lWidth = *(long*) &abtHeader[4];
	long				lHeight = *(long*) &abtHeader[8];
	DWORD				dwDstSize = *(DWORD*) &abtHeader[16];
	DWORD				dwSrcSize = *(DWORD*) &abtHeader[20];

	// 圧縮データの読み込み

	YCMemory<BYTE>		clmSrc( dwSrcSize );

	pclArc->Read( &clmSrc[0], dwSrcSize );

	// 解凍後のバッファの確保

	YCMemory<BYTE>		clmDst( dwDstSize );

	// LZSSの解凍

	CLZSS				clLZSS;

	clLZSS.Decomp( &clmDst[0], dwDstSize, &clmSrc[0], dwSrcSize, 4096, 4078, 3 );

	// 出力

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, 32 );
	clImage.WriteReverse( &clmDst[0], dwDstSize );
	clImage.Close();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	TGAのデコード

BOOL	CTaskForce::DecodeTGA(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->name.GetFileExt() != _T(".tga") )
	{
		return	FALSE;
	}

	// データの読み込み

	DWORD				dwSrcSize = pstFileInfo->sizeCmp;

	YCMemory<BYTE>		clmSrc( dwSrcSize );

	pclArc->Read( &clmSrc[0], dwSrcSize );

	if( pstFileInfo->format == _T("LZ") )
	{
		// 圧縮されている

		DWORD				dwDstSize = pstFileInfo->sizeOrg;

		YCMemory<BYTE>		clmDst( dwDstSize );

		// LZSSの解凍

		CLZSS				clLZSS;

		clLZSS.Decomp( &clmDst[0], dwDstSize, &clmSrc[0], dwSrcSize, 4096, 4078, 3 );

		// 出力

		CTga				clTGA;

		clTGA.Decode( pclArc, &clmDst[0], dwDstSize );
	}
	else
	{
		// 無圧縮

		CTga				clTGA;

		clTGA.Decode( pclArc, &clmSrc[0], dwSrcSize );
	}

	return	TRUE;
}
