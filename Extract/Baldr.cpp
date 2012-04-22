#include "stdafx.h"
#include "../ExtractBase.h"
#include "Baldr.h"

BOOL	CBaldr::Mount(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".pac") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "PAC", 3 ) != 0 )
	{
		return	FALSE;
	}

	if( memcmp( &pclArc->GetHed()[72], "\0\0\0\0", 4 ) != 0 )
	{
		return	FALSE;
	}

	// ファイル数取得

	DWORD				dwFiles;

	pclArc->Seek( 4, FILE_BEGIN );
	pclArc->Read( &dwFiles, 4 );

	// フラグ取得

	DWORD				dwFlags;

	pclArc->Read( &dwFlags, 4 );

	// 圧縮フォーマットの取得

	YCString				clsFormat;

	switch( dwFlags )
	{
	case	0:
		// 無圧縮

		break;

	case	1:
		// LZSS

		clsFormat = _T("LZ");
		break;

	case	2:
		// 不明

		break;

	case	3:
		// ZLIB

		clsFormat = _T("zlib");
		break;
	}

	// インデックス取得

	YCMemory<SPACFileInfo>	clmpacfiIndex( dwFiles );

	pclArc->Read( &clmpacfiIndex[0], (sizeof(SPACFileInfo) * dwFiles) );

	// ファイル情報取得

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// ファイル名取得

		char				szFileName[65];

		memcpy( szFileName, clmpacfiIndex[i].szFileName, 64 );
		szFileName[64] = '\0';

		if( strlen( szFileName ) <= 4 )
		{
			pclArc->SeekHed();
			return	FALSE;
		}

		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.sizeOrg = clmpacfiIndex[i].dwFileSize;
		stFileInfo.sizeCmp = clmpacfiIndex[i].dwCompFileSize;
		stFileInfo.start = clmpacfiIndex[i].dwOffset;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;
		stFileInfo.format = clsFormat;

//		if( (stFileInfo.sizeOrg != infFile.sizeCmp) && (lstrcmp(PathFindExtension(infFile.name), _T(".wav")) != 0))
//			infFile.format = _T("LZ");

		pclArc->AddFileInfo( stFileInfo );
	}

	return	TRUE;
}
