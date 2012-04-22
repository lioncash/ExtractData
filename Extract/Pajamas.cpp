
#include	"stdafx.h"
#include	"../ExtractBase.h"
#include	"../Image.h"
#include	"Pajamas.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CPajamas::Mount(
	CArcFile*			pclArc							// マウント
	)
{
	if( MountDat1( pclArc ) )
	{
		return	TRUE;
	}

	if( MountDat2( pclArc ) )
	{
		return	TRUE;
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	GAMEDAT PACKのマウント

BOOL	CPajamas::MountDat1(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( (pclArc->GetArcExten() != _T(".dat")) && (pclArc->GetArcExten() != _T(".pak")) )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "GAMEDAT PACK", 12 ) != 0 )
	{
		return	FALSE;
	}

	// ファイル数取得

	DWORD				dwFiles;

	pclArc->SeekHed( 12 );
	pclArc->Read( &dwFiles, 4 );

	// ファイル数からインデックスサイズ取得

	DWORD				dwIndexSize = dwFiles * 24;

	// インデックス取得

	YCMemory<BYTE>		clmbtIndex( dwIndexSize );
	DWORD				dwIndexPtr = 0;

	pclArc->Read( &clmbtIndex[0], dwIndexSize );

	// ファイル情報のインデックス取得

	DWORD				dwFileNameIndexSize = dwFiles << 4;

	BYTE*				pbtFileInfoIndex = &clmbtIndex[dwFileNameIndexSize];

	// オフセット取得(開始アドレスが0から始まっているため補正用のオフセットが必要)

	DWORD				dwOffset = 16 + dwIndexSize;

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// ファイル名取得

		TCHAR				szFileName[16];

		memcpy( szFileName, &clmbtIndex[dwIndexPtr], 16 );

		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.start = *(DWORD*) &pbtFileInfoIndex[0] + dwOffset;
		stFileInfo.sizeCmp = *(DWORD*) &pbtFileInfoIndex[4];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );

		dwIndexPtr += 16;
		pbtFileInfoIndex += 8;
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	GAMEDAT PAC2のマウント

BOOL	CPajamas::MountDat2(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".dat") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "GAMEDAT PAC2", 12 ) != 0 )
	{
		return	FALSE;
	}

	// ファイル数取得

	DWORD				dwFiles;

	pclArc->SeekHed( 12 );
	pclArc->Read( &dwFiles, 4 );

	// ファイル数からインデックスサイズ取得

	DWORD				dwIndexSize = dwFiles * 40;

	// インデックス取得

	YCMemory<BYTE>		clmbtIndex( dwIndexSize );
	DWORD				dwIndexPtr = 0;

	pclArc->Read( &clmbtIndex[0], dwIndexSize );

	// ファイル情報のインデックス取得

	DWORD				dwFileNameIndexSize = dwFiles << 5;

	BYTE*				pbtFileInfoIndex = &clmbtIndex[dwFileNameIndexSize];

	// オフセット取得(開始アドレスが0から始まっているため補正用のオフセットが必要)

	DWORD				dwOffset = 16 + dwIndexSize;

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// ファイル名取得

		TCHAR				szFileName[32];

		memcpy( szFileName, &clmbtIndex[dwIndexPtr], 32 );

		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.start = *(DWORD*) &pbtFileInfoIndex[0] + dwOffset;
		stFileInfo.sizeCmp = *(DWORD*) &pbtFileInfoIndex[4];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );

		dwIndexPtr += 32;
		pbtFileInfoIndex += 8;
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード

BOOL	CPajamas::Decode(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( DecodeEPA( pclArc ) )
	{
		return	TRUE;
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	EPAのデコード

BOOL	CPajamas::DecodeEPA(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->format != _T("EPA") )
	{
		return	FALSE;
	}

	// ヘッダ読み込み

	BYTE				abtHeader[16];

	pclArc->Read( abtHeader, sizeof(abtHeader) );

	// 差分フラグ、色数、幅、高さ取得

	BYTE				btDiffFlag = abtHeader[3];
	WORD				wBpp = abtHeader[4];
	long				lWidth = *(long*) &abtHeader[8];
	long				lHeight = *(long*) &abtHeader[12];

	switch( wBpp )
	{
		case	0:
			wBpp = 8;
			break;

		case	1:
			wBpp = 24;
			break;

		case	2:
			wBpp = 32;
			break;

		case	4:
			wBpp = 8;
			break;
	}

	switch( btDiffFlag )
	{
	case	1:
		// 通常

		break;

	case	2:
		// 差分

		pclArc->SeekCur( 40 );
		break;

	default:
		// 不明

		pclArc->SeekHed( pstFileInfo->start );
		return	FALSE;
	}

	// パレットの読み込み

	BYTE				abtPallet[768];

	if( wBpp == 8 )
	{
		pclArc->Read( abtPallet, sizeof(abtPallet) );
	}

	// EPAデータ読み込み

	DWORD				dwSrcSize = pstFileInfo->sizeCmp - 16;

	if( wBpp == 8 )
	{
		dwSrcSize -= sizeof(abtPallet);
	}

	YCMemory<BYTE>		clmbtSrc( dwSrcSize );

	pclArc->Read( &clmbtSrc[0], dwSrcSize );

	// BMPデータを格納する領域確保

	DWORD				dwDstSize = lWidth * lHeight * (wBpp >> 3);

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	ZeroMemory( &clmbtDst[0], dwDstSize );

	// EPAを解凍

	DecompEPA( &clmbtDst[0], dwDstSize, &clmbtSrc[0], dwSrcSize, lWidth );

	// 出力

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp, abtPallet, sizeof(abtPallet) );
	clImage.WriteCompoBGRAReverse( &clmbtDst[0], dwDstSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	EPAの解凍

BOOL	CPajamas::DecompEPA(
	void*				pvDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const void*			pvSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth							// 横幅
	)
{
	const BYTE*			pbtSrc = (const BYTE*) pvSrc;
	BYTE*				pbtDst = (BYTE*) pvDst;

	DWORD				dwSrcPtr = 0;
	DWORD				dwDstPtr = 0;

	// オフセットテーブル

	const DWORD			adwOffsets[16] =
	{
		0, 1, lWidth, lWidth + 1,
		2, lWidth - 1, lWidth << 1, 3,
		(lWidth << 1) + 2, lWidth + 2, (lWidth << 1) + 1, (lWidth << 1) - 1,
		(lWidth << 1) - 2, lWidth - 2, lWidth * 3, 4
	};

	// 解凍

	while( (dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) )
	{
		BYTE				btCode = pbtSrc[dwSrcPtr++];
		DWORD				dwLength = btCode & 0x07;

		if( btCode & 0xF0 )
		{
			if( btCode & 0x08 )
			{
				dwLength = (dwLength << 8) + pbtSrc[dwSrcPtr++];
			}

			if( dwLength != 0 )
			{
				btCode >>= 4;

				DWORD				dwBack = dwDstPtr - adwOffsets[btCode];

				if( (dwDstPtr + dwLength) > dwDstSize )
				{
					// 出力バッファを超えてしまう

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

	return	TRUE;
}
