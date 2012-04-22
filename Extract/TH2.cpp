
#include	"stdafx.h"
#include	"../ExtractBase.h"
#include	"../Arc/LZSS.h"
#include	"../Image.h"
#include	"../Tga.h"
#include	"TH2.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CTH2::Mount(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( MountKCAP( pclArc ) )
	{
		return	TRUE;
	}

	if( MountLAC( pclArc ) )
	{
		return	TRUE;
	}

	if( MountDpl( pclArc ) )
	{
		return	TRUE;
	}

	if( MountWMV( pclArc ) )
	{
		return	TRUE;
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	KCAPのマウント

BOOL	CTH2::MountKCAP(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten().CompareNoCase( _T(".pak") ) != 0 )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "KCAP", 4 ) != 0 )
	{
		return	FALSE;
	}

	// ファイル数取得

	DWORD				dwFiles;

	pclArc->SeekHed( 4 );
	pclArc->Read( &dwFiles, 4 );

	// ファイル数からインデックスサイズ取得

	DWORD				dwIndexSize = (dwFiles * 36);

	// インデックス取得

	YCMemory<BYTE>		clmIndex( dwIndexSize );

	pclArc->Read( &clmIndex[0], dwIndexSize );

	DWORD				dwIndexPtr = 0;

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		DWORD				dwType = *(DWORD*) &clmIndex[dwIndexPtr + 0];

		// ごみファイルスキップ

		if( dwType == 0xCCCCCCCC )
		{
			dwIndexPtr += 36;
			continue;
		}

		// ファイル名取得

		char				szFileName[25];

		memcpy( szFileName, &clmIndex[dwIndexPtr + 4], 24 );

		szFileName[24] = '\0';

		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.start = *(DWORD*) &clmIndex[dwIndexPtr + 28];
		stFileInfo.sizeCmp = *(DWORD*) &clmIndex[dwIndexPtr + 32];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;
		stFileInfo.title = _T("TH2");

		if( dwType == 1 )
		{
			stFileInfo.format = _T("LZ");
		}

		pclArc->AddFileInfo( stFileInfo );

		dwIndexPtr += 36;
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	LACのマウント

BOOL	CTH2::MountLAC(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten().CompareNoCase( _T(".pak") ) != 0 )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "LAC", 3 ) != 0 )
	{
		return	FALSE;
	}

	// ファイル数取得

	DWORD				dwFiles;

	pclArc->SeekHed( 4 );
	pclArc->Read( &dwFiles, 4 );

	// ファイル数からインデックスサイズ取得

	DWORD				dwIndexSize = (dwFiles * 40);

	// インデックス取得

	YCMemory<BYTE>		clmIndex( dwIndexSize );

	pclArc->Read( &clmIndex[0], dwIndexSize );

	DWORD				dwIndexPtr = 0;

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// ファイル名取得

		char				szFileName[33];

		memcpy( szFileName, &clmIndex[dwIndexPtr + 0], 32 );

		szFileName[32] = '\0';

		for( int j = 0 ; j < lstrlen( szFileName ) ; j++ )
		{
			szFileName[j] ^= 0xFF;
		}

		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.sizeCmp = *(DWORD*) &clmIndex[dwIndexPtr + 32];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.start = *(DWORD*) &clmIndex[dwIndexPtr + 36];
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );

		dwIndexPtr += 40;
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	

BOOL	CTH2::MountDpl(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".a") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "\x1E\xAF", 2 ) != 0 )
	{
		return	FALSE;
	}

	// ファイル数取得

	WORD				wFiles;

	pclArc->SeekHed( 2 );
	pclArc->Read( &wFiles, 2 );

	// ファイル数からインデックスサイズ取得

	DWORD				dwIndexSize = (wFiles << 5);

	// インデックス取得

	YCMemory<BYTE>		clmIndex( dwIndexSize );

	pclArc->Read( &clmIndex[0], dwIndexSize );

	DWORD				dwOffset = (dwIndexSize + 4);
	DWORD				dwIndexPtr = 0;

	for( WORD i = 0 ; i < wFiles ; i++ )
	{
		// ファイル名取得

		char				szFileName[25];

		memcpy( szFileName, &clmIndex[dwIndexPtr + 0], 24 );

		szFileName[24] = '\0';

		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.sizeCmp = *(DWORD*) &clmIndex[dwIndexPtr + 24];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.start = *(DWORD*) &clmIndex[dwIndexPtr + 28] + dwOffset;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );

		dwIndexPtr += 32;
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	WMVのマウント

BOOL	CTH2::MountWMV(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".wmv") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "\x00\x00\x00\x00\x00\x00\x00\x00\xA6\xD9\x00\xAA\x00\x62\xCE\x6C", 16 ) != 0 )
	{
		return	FALSE;
	}

	return	pclArc->Mount();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード

BOOL	CTH2::Decode(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( DecodeWMV( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodeEtc( pclArc ) )
	{
		return	TRUE;
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	WMVのデコード

BOOL	CTH2::DecodeWMV(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pclArc->GetArcExten() != _T(".wmv") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "\x00\x00\x00\x00\x00\x00\x00\x00\xA6\xD9\x00\xAA\x00\x62\xCE\x6C", 16 ) != 0 )
	{
		return	FALSE;
	}

	// バッファの取得

	DWORD				dwBufferSize = pclArc->GetBufSize();

	YCMemory<BYTE>		clmBuffer( dwBufferSize );

	// 出力

	pclArc->OpenFile();

	pclArc->SeekCur( 8 );
	pclArc->WriteFile( "\x30\x26\xB2\x75\x8E\x66\xCF\x11", 8 );

	pclArc->ReadWrite( (pstFileInfo->sizeCmp - 8) );

	pclArc->CloseFile();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	その他ファイルのデコード

BOOL	CTH2::DecodeEtc(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->title != _T("TH2") )
	{
		return	FALSE;
	}

	if( (pstFileInfo->format != _T("LZ")) && (pstFileInfo->format != _T("TGA")) )
	{
		return	FALSE;
	}

	YCMemory<BYTE>		clmDst;
	DWORD				dwDstSize;

	// LZ展開

	if( pstFileInfo->format == _T("LZ") )
	{
		// 入力サイズ取得

		DWORD				dwSrcSize = (pstFileInfo->sizeCmp - 8);

		// 出力サイズ取得

		pclArc->SeekCur( 4 );
		pclArc->Read( &dwDstSize, 4 );

		// バッファ確保

		YCMemory<BYTE>		clmSrc( dwSrcSize );

		clmDst.resize( dwDstSize );

		// 読み込み

		pclArc->Read( &clmSrc[0], dwSrcSize );

		// LZSS解凍

		CLZSS				clLZSS;

		clLZSS.Decomp( &clmDst[0], dwDstSize, &clmSrc[0], dwSrcSize, 4096, 4078, 3 );
	}
	else
	{
		// 無圧縮

		dwDstSize = pstFileInfo->sizeOrg;

		clmDst.resize( dwDstSize );

		pclArc->Read( &clmDst[0], dwDstSize );
	}

	YCString			clsFileExt = pstFileInfo->name.GetFileExt().MakeLower();

	if( clsFileExt == _T(".tga") )
	{
		// TGA

		CTga				clTGA;

		clTGA.Decode( pclArc, &clmDst[0], dwDstSize );
	}
	else if( clsFileExt == _T(".bmp") )
	{
		// BMP

		CImage				clImage;

		clImage.Init( pclArc, &clmDst[0] );
		clImage.Write( dwDstSize );
	}
	else
	{
		// その他

		pclArc->OpenFile();
		pclArc->WriteFile( &clmDst[0], dwDstSize );
	}

	return	TRUE;
}
