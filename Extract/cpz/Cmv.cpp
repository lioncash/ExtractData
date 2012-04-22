
#include	"stdafx.h"
#include	"../../ExtractBase.h"
#include	"../../Image.h"
#include	"JBP1.h"
#include	"Cmv.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CCmv::Mount(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".cmv") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "CMV", 3 ) != 0 )
	{
		return	FALSE;
	}

	// ヘッダ読み込み

	BYTE				abtHeader[44];

	pclArc->Read( abtHeader, sizeof(abtHeader) );

	// オフセット取得

	DWORD				dwOffset = *(DWORD*) &abtHeader[4];

	// インデックスサイズ取得

	DWORD				dwIndexSize = *(DWORD*) &abtHeader[4] - 44;

	// ファイル数取得

	DWORD				dwFiles = *(DWORD*) &abtHeader[16] + 1;

	// インデックス取得

	YCMemory<BYTE>		clmbtIndex( dwIndexSize );
	DWORD				dwIndexPtr = 0;

	pclArc->Read( &clmbtIndex[0], dwIndexSize );

	// アーカイブ名の取得

	TCHAR				szArcName[_MAX_FNAME];

	lstrcpy( szArcName, pclArc->GetArcName() );
	PathRenameExtension( szArcName, _T("_") );

	// ファイル情報の取得

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		TCHAR				szFileExt[_MAX_EXT];
		DWORD				dwType = *(DWORD*) &clmbtIndex[12];

		switch( dwType )
		{
			case	0:
				// Ogg Vorbis

				lstrcpy( szFileExt, _T(".ogg") );
				break;

			case	2:
				// JBP

				lstrcpy( szFileExt, _T(".jbp") );
				break;
		}

		// 連番ファイル名の作成

		TCHAR				szFileName[_MAX_FNAME];

		_stprintf( szFileName, _T("%s%06d%s"), szArcName, i, szFileExt );

		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.sizeCmp = *(DWORD*) &clmbtIndex[dwIndexPtr + 4];
		stFileInfo.sizeOrg = *(DWORD*) &clmbtIndex[dwIndexPtr + 8];
		stFileInfo.start = *(DWORD*) &clmbtIndex[dwIndexPtr + 16] + dwOffset;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );

		dwIndexPtr += 20;
	}

	return	TRUE;
}

BOOL CCmv::Decode(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->format != _T("JBP"))
		return FALSE;

	YCMemory<BYTE> src(pInfFile->sizeCmp);
	pclArc->Read(&src[0], pInfFile->sizeCmp);

	LONG width = *(LPWORD)&src[0x10];
	LONG height = *(LPWORD)&src[0x12];
	WORD bpp = *(LPWORD)&src[0x14];
	WORD colors = bpp >> 3;

	DWORD dstSize = width * height * colors;
	YCMemory<BYTE> dst(dstSize);

	if( memcmp( &src[0], "JBP1", 4 ) == 0 )
	{
		CJBP1 jbp1;
		jbp1.Decomp(&dst[0], &src[0]);

		CImage image;
		image.Init(pclArc, width, height, bpp);
		image.WriteReverse(&dst[0], dstSize);
	}
	else
	{
		pclArc->OpenFile();
		pclArc->WriteFile( &src[0], pInfFile->sizeCmp );
	}

	return TRUE;
}
