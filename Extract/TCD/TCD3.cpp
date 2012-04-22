
#include	"stdafx.h"
#include	"../../Image.h"
#include	"../../Ogg.h"
#include	"TCD3.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CTCD3::Mount(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".TCD") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "TCD3", 4 ) != 0 )
	{
		return	FALSE;
	}

	pclArc->SeekHed( 4 );

	// ファイル数の取得

	DWORD				dwFiles;

	pclArc->Read( &dwFiles, 4 );

	// インデックス情報の取得

	STCD3IndexInfo		astTCD3IndexInfo[5];

	pclArc->Read( astTCD3IndexInfo, (sizeof(STCD3IndexInfo) * 5) );

	// キーテーブルの作成

	static BYTE			abtKey[5] = {
		0xB7, 0x39, 0x24, 0x8D, 0x8D
	};

	// 拡張子テーブルの作成

	static YCString		aclsFileExt[5] =
	{
		_T(".tct"), _T(".tsf"), _T(".spd"), _T(".ogg"), _T(".wav")
	};

	// インデックスの読み込み

	for( DWORD dwFileType = 0 ; dwFileType < 5 ; dwFileType++ )
	{
		if( astTCD3IndexInfo[dwFileType].dwFileSize == 0 )
		{
			// インデックスが存在しない

			continue;
		}

		// インデックスへ移動

		pclArc->SeekHed( astTCD3IndexInfo[dwFileType].dwIndexOffset );

		// フォルダ名

		DWORD				dwAllDirNameLength = (astTCD3IndexInfo[dwFileType].dwDirNameLength * astTCD3IndexInfo[dwFileType].dwDirCount);

		YCMemory<BYTE>		clmbtAllDirName( dwAllDirNameLength );

		pclArc->Read( &clmbtAllDirName[0], dwAllDirNameLength );

		// フォルダ名の復号

		for( DWORD i = 0 ; i < dwAllDirNameLength ; i++ )
		{
			clmbtAllDirName[i] -= abtKey[dwFileType];
		}

		// フォルダ情報の取得

		YCMemory<STCD3DirInfo>	clmstTCD3DirInfo( astTCD3IndexInfo[dwFileType].dwDirCount );

		pclArc->Read( &clmstTCD3DirInfo[0], (sizeof(STCD3DirInfo) * astTCD3IndexInfo[dwFileType].dwDirCount) );

		// ファイル名

		DWORD				dwAllFileNameLength = (astTCD3IndexInfo[dwFileType].dwFileNameLength * astTCD3IndexInfo[dwFileType].dwFileCount);

		YCMemory<BYTE>		clmbtAllFileName( dwAllFileNameLength );

		pclArc->Read( &clmbtAllFileName[0], dwAllFileNameLength );

		// ファイル名の復号

		for( DWORD i = 0 ; i < dwAllFileNameLength ; i++ )
		{
			clmbtAllFileName[i] -= abtKey[dwFileType];
		}

		// ファイルへのオフセット

		DWORD				dwAllFileOffsetLength = (astTCD3IndexInfo[dwFileType].dwFileCount + 1);

		YCMemory<DWORD>		clmdwAllFileOffset( dwAllFileOffsetLength );

		pclArc->Read( &clmdwAllFileOffset[0], (sizeof(DWORD) * dwAllFileOffsetLength) );

		// 情報の格納

		for( DWORD dwDir = 0 ; dwDir < astTCD3IndexInfo[dwFileType].dwDirCount ; dwDir++ )
		{
			// フォルダ名の取得

			TCHAR				szDirName[_MAX_DIR];

			memcpy( szDirName, &clmbtAllDirName[astTCD3IndexInfo[dwFileType].dwDirNameLength * dwDir], astTCD3IndexInfo[dwFileType].dwDirNameLength );

			for( DWORD dwFile = 0 ; dwFile < clmstTCD3DirInfo[dwDir].dwFileCount ; dwFile++ )
			{
				// ファイル名の取得

				TCHAR				szFileName[_MAX_FNAME];

				memcpy( szFileName, &clmbtAllFileName[clmstTCD3DirInfo[dwDir].dwFileNameOffset + astTCD3IndexInfo[dwFileType].dwFileNameLength * dwFile], astTCD3IndexInfo[dwFileType].dwFileNameLength );
				szFileName[astTCD3IndexInfo[dwFileType].dwFileNameLength] = _T('\0');

				// フォルダ名 + ファイル名 + 拡張子

				TCHAR				szFullName[_MAX_PATH];

				_stprintf( szFullName, _T("%s\\%s%s"), szDirName, szFileName, aclsFileExt[dwFileType] );

				// 情報の格納

				SFileInfo			stFileInfo;

				stFileInfo.name = szFullName;
				stFileInfo.start = clmdwAllFileOffset[clmstTCD3DirInfo[dwDir].dwFileOffset + dwFile + 0];
				stFileInfo.end = clmdwAllFileOffset[clmstTCD3DirInfo[dwDir].dwFileOffset + dwFile + 1];
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

BOOL	CTCD3::DecompRLE2(
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
		WORD				wWork = *(WORD*)&pbtSrc[dwSrcHeaderPtr];

		dwSrcHeaderPtr += 2;

		WORD				wType = wWork >> 14;
		WORD				wLength = wWork & 0x3FFF;

		if( (dwDstPtr + (wLength * 4)) > dwDstSize )
		{
			MessageBox( NULL, _T("RLE2の解凍に必要な出力バッファが足りません。多分落ちます。"), _T("エラー"), 0 );
		}

		switch( wType )
		{
		case	0:
			// 0で埋める

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

			if( (dwSrcDataPtr + (wLength * 3)) > dwSrcSize )
			{
				MessageBox( NULL, _T("RLE2の解凍に必要な入力バッファが足りません。多分落ちます。"), _T("エラー"), 0 );
			}

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
			// アルファ値をヘッダから取得

			if( (dwSrcDataPtr + (wLength * 3)) > dwSrcSize )
			{
				MessageBox( NULL, _T("RLE2の解凍に必要な入力バッファが足りません。多分落ちます。"), _T("エラー"), 0 );
			}

			if( (dwSrcHeaderPtr + wLength) > dwOffset )
			{
				MessageBox( NULL, _T("RLE2の解凍に必要な入力バッファが足りません。多分落ちます。"), _T("エラー"), 0 );
			}

			for( DWORD i = 0 ; i < wLength ; i++ )
			{
				for( DWORD j = 0 ; j < 3 ; j++ )
				{
					pbtDst[dwDstPtr++] = pbtSrc[dwSrcDataPtr++];
				}

				pbtDst[dwDstPtr++] = pbtSrc[dwSrcHeaderPtr++];
			}

			break;
		}
	}

	return	TRUE;
}
