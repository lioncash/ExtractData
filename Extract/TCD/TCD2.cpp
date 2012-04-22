
#include	"stdafx.h"
#include	"../../Image.h"
#include	"../../Ogg.h"
#include	"TCD2.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CTCD2::Mount(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".TCD") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "TCD2", 4 ) != 0 )
	{
		return	FALSE;
	}

	pclArc->SeekHed( 4 );

	// ファイル数の取得

	DWORD				dwFiles;

	pclArc->Read( &dwFiles, 4 );

	// インデックス情報の取得

	STCD2IndexInfo		astTCD2IndexInfo[5];

	pclArc->Read( astTCD2IndexInfo, (sizeof(STCD2IndexInfo) * 5) );

	// キーテーブルの作成

	static BYTE			abtKey[5] =
	{
		0x1F, 0x61, 0x43, 0x76, 0x76
	};

	// 拡張子テーブルの作成

	static YCString		aclsFileExt[5] =
	{
		_T(".tct"), _T(".tsf"), _T(".spd"), _T(".ogg"), _T(".wav")
	};

	// インデックスの読み込み

	for( DWORD dwFileType = 0 ; dwFileType < 5 ; dwFileType++ )
	{
		if( astTCD2IndexInfo[dwFileType].dwFileSize == 0 )
		{
			// インデックスが存在しない

			continue;
		}

		// インデックスへ移動

		pclArc->SeekHed( astTCD2IndexInfo[dwFileType].dwIndexOffset );

		// フォルダ名

		DWORD				dwAllDirNameLength = astTCD2IndexInfo[dwFileType].dwDirNameLength;

		YCMemory<BYTE>		clmbtAllDirName( dwAllDirNameLength );

		pclArc->Read( &clmbtAllDirName[0], dwAllDirNameLength );

		// フォルダ名の復号

		for( DWORD i = 0 ; i < dwAllDirNameLength ; i++ )
		{
			clmbtAllDirName[i] -= abtKey[dwFileType];
		}

		// フォルダ情報の取得

		YCMemory<STCD2DirInfo>	clmstTCD2DirInfo( astTCD2IndexInfo[dwFileType].dwDirCount );

		pclArc->Read( &clmstTCD2DirInfo[0], (sizeof(STCD2DirInfo) * astTCD2IndexInfo[dwFileType].dwDirCount) );

		// ファイル名

		DWORD				dwAllFileNameLength = astTCD2IndexInfo[dwFileType].dwFileNameLength;

		YCMemory<BYTE>		clmbtAllFileName( dwAllFileNameLength );

		pclArc->Read( &clmbtAllFileName[0], dwAllFileNameLength );

		// ファイル名の復号

		for( DWORD i = 0 ; i < dwAllFileNameLength ; i++ )
		{
			clmbtAllFileName[i] -= abtKey[dwFileType];
		}

		// ファイルへのオフセット

		DWORD				dwAllFileOffsetLength = (astTCD2IndexInfo[dwFileType].dwFileCount + 1);

		YCMemory<DWORD>		clmdwAllFileOffset( dwAllFileOffsetLength );

		pclArc->Read( &clmdwAllFileOffset[0], (sizeof(DWORD) * dwAllFileOffsetLength) );

		// 情報の格納

		DWORD				dwDirNamePtr = 0;

		for( DWORD dwDir = 0 ; dwDir < astTCD2IndexInfo[dwFileType].dwDirCount ; dwDir++ )
		{
			// フォルダ名の取得

			char				szDirName[_MAX_DIR];

			strcpy( szDirName, (char*) &clmbtAllDirName[dwDirNamePtr] );

			dwDirNamePtr += strlen( szDirName ) + 1;

			DWORD				dwFileNamePtr = 0;

			for( DWORD dwFile = 0 ; dwFile < clmstTCD2DirInfo[dwDir].dwFileCount ; dwFile++ )
			{
				// ファイル名の取得

				char				szFileName[_MAX_FNAME];

				strcpy( szFileName, (char*) &clmbtAllFileName[clmstTCD2DirInfo[dwDir].dwFileNameOffset + dwFileNamePtr] );

				dwFileNamePtr += strlen( szFileName ) + 1;

				// フォルダ名 + ファイル名 + 拡張子

				TCHAR				szFullName[_MAX_PATH];

				_stprintf( szFullName, _T("%s\\%s%s"), szDirName, szFileName, aclsFileExt[dwFileType] );

				// 情報の格納

				SFileInfo			stFileInfo;

				stFileInfo.name = szFullName;
				stFileInfo.start = clmdwAllFileOffset[clmstTCD2DirInfo[dwDir].dwFileOffset + dwFile + 0];
				stFileInfo.end = clmdwAllFileOffset[clmstTCD2DirInfo[dwDir].dwFileOffset + dwFile + 1];
				stFileInfo.sizeCmp = stFileInfo.end - stFileInfo.start;
				stFileInfo.sizeOrg = stFileInfo.sizeCmp;

				pclArc->AddFileInfo( stFileInfo );
			}
		}
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	RLEの解凍(タイプ2)

BOOL	CTCD2::DecompRLE2(
	void*				pvDst,							// 格納先
	DWORD				dwDstSize,						// 格納サイズ
	const void*			pvSrc,							// 入力データ
	DWORD				dwSrcSize						// 入力データサイズ
	)
{
	const BYTE*			pbtSrc = (const BYTE*) pvSrc;
	BYTE*				pbtDst = (BYTE*) pvDst;

	DWORD				dwOffset = *(DWORD*) &pbtSrc[0];
	DWORD				dwPixelCount = *(DWORD*) &pbtSrc[4];

	DWORD				dwSrcHeaderPtr = 8;
	DWORD				dwSrcDataPtr = dwOffset;
	DWORD				dwDstPtr = 0;

	while( (dwSrcHeaderPtr < dwOffset) && (dwSrcDataPtr < dwSrcSize) && (dwDstPtr < dwDstSize) )
	{
		BYTE				btWork = pbtSrc[dwSrcHeaderPtr++];
		WORD				wLength;

		switch( btWork )
		{
		case	0:
			// 0で埋める

			wLength = pbtSrc[dwSrcHeaderPtr++] + 1;

			for( DWORD i = 0 ; i < wLength ; i++ )
			{
				for( DWORD j = 0 ; j < 4 ; j++ )
				{
					pbtDst[dwDstPtr++] = 0x00;
				}
			}

			break;

		case	1:
			// アルファ値0xFF

			wLength = pbtSrc[dwSrcHeaderPtr++] + 1;

			for( DWORD i = 0 ; i < wLength ; i++ )
			{
				for( DWORD j = 0 ; j < 3 ; j++ )
				{
					pbtDst[dwDstPtr++] = pbtSrc[dwSrcDataPtr++];
				}

				pbtDst[dwDstPtr++] = 0xFF;
			}

			break;

		default:
			// アルファ値0x01～0xFE

			for( DWORD j = 0 ; j < 3 ; j++ )
			{
				pbtDst[dwDstPtr++] = pbtSrc[dwSrcDataPtr++];
			}

			pbtDst[dwDstPtr++] = ~(btWork - 1);
		}
	}

	return	TRUE;
}
