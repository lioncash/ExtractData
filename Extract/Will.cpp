
#include	"stdafx.h"
#include	"../ExtractBase.h"
#include	"../Image.h"
#include	"Will.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CWill::Mount(
	CArcFile*			pclArc							// マウント
	)
{
	if( lstrcmpi( pclArc->GetArcExten(), _T(".arc") ) != 0 )
	{
		return	FALSE;
	}

	BOOL				bMatch = FALSE;

	static const char*	apszHeader[] =
	{
		"OGG", "WSC", "ANM", "MSK", "WIP", "TBL", "SCR"
	};

	for( DWORD i = 0 ; i < _countof( apszHeader ) ; i++ )
	{
		if( memcmp( &pclArc->GetHed()[4], apszHeader[i], 4 ) == 0 )
		{
			bMatch = TRUE;
			break;
		}
	}

	if( !bMatch )
	{
		return	FALSE;
	}

	// ファイルの種類数取得

	DWORD				dwFileFormats;

	pclArc->Read( &dwFileFormats, 4 );

	// ファイルフォーマットインデックス取得

	DWORD				dwFormatIndexSize = 12 * dwFileFormats;

	YCMemory<BYTE>		clmbtFormatIndex( dwFormatIndexSize );
	DWORD				dwFormatIndexPtr = 0;

	pclArc->Read( &clmbtFormatIndex[0], dwFormatIndexSize );

	// インデックスサイズ取得

	DWORD				dwIndexSize = 0;

	for( DWORD i = 0 ; i < dwFileFormats ; i++ )
	{
		dwIndexSize += *(DWORD*) &clmbtFormatIndex[12 * i + 4] * 17;
	}

	// インデックス取得

	YCMemory<BYTE>		clmbtIndex( dwIndexSize );
	DWORD				dwIndexPtr = 0;

	pclArc->Read( &clmbtIndex[0], dwIndexSize );

	// ファイル情報の取得

	std::vector<SFileInfo>	vcFileInfo;
	std::vector<SFileInfo>	vcMaskFileInfo;
	std::vector<SFileInfo>	vcNotMaskFileInfo;

	for( DWORD i = 0 ; i < dwFileFormats ; i++ )
	{
		// ファイルの種類から拡張子取得

		TCHAR				szFileExt[8];

		memcpy( szFileExt, &clmbtFormatIndex[dwFormatIndexPtr], 4 );
		szFileExt[4] = '\0';

		::CharLower( szFileExt );

		// ファイル情報の取得

		DWORD				dwFiles = *(DWORD*) &clmbtFormatIndex[dwFormatIndexPtr + 4];

		for( DWORD j = 0 ; j < dwFiles; j++ )
		{
			// ファイル名取得

			char				szFileTitle[16];

			memcpy( szFileTitle, &clmbtIndex[dwIndexPtr], 9 );
			szFileTitle[9] = '\0';

			TCHAR				szFileName[32];

			_stprintf( szFileName, _T("%s.%s"), szFileTitle, szFileExt );

			// ファイル情報リストに追加

			SFileInfo			stFileInfo;

			stFileInfo.name = szFileName;
			stFileInfo.sizeCmp = *(DWORD*) &clmbtIndex[dwIndexPtr + 9];
			stFileInfo.sizeOrg = stFileInfo.sizeCmp;
			stFileInfo.start = *(DWORD*) &clmbtIndex[dwIndexPtr + 13];
			stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

			if( lstrcmp( szFileExt, _T("msk") ) == 0 )
			{
				// マスク画像

				vcMaskFileInfo.push_back( stFileInfo );
			}
			else
			{
				vcFileInfo.push_back( stFileInfo );
			}

			dwIndexPtr += 17;
		}

		dwFormatIndexPtr += 12;
	}

	// ファイル名でソート

	std::sort( vcFileInfo.begin(), vcFileInfo.end(), CArcFile::CompareForFileInfo );

	// マスク画像を除去したファイル情報の取得

	for( size_t i = 0 ; i < vcMaskFileInfo.size() ; i++ )
	{
		SFileInfo*			pstsiMask = &vcMaskFileInfo[i];

		// 合成するファイル名の取得

		TCHAR				szTargetName[_MAX_FNAME];

		lstrcpy( szTargetName, pstsiMask->name );

		PathRenameExtension( szTargetName, _T(".wip") );

		// 合成するファイル情報の取得

		SFileInfo*			pstsiTarget = NULL;

		pstsiTarget = pclArc->SearchForFileInfo( vcFileInfo, szTargetName );

		if( pstsiTarget != NULL )
		{
			// マスク画像であると断定

			pstsiTarget->starts.push_back( pstsiMask->start );
			pstsiTarget->sizesCmp.push_back( pstsiMask->sizeCmp );
			pstsiTarget->sizesOrg.push_back( pstsiMask->sizeOrg );

			// 進捗状況更新

			pclArc->GetProg()->UpdatePercent( pstsiMask->sizeCmp );
		}
		else
		{
			// マスク画像ではない

			vcNotMaskFileInfo.push_back( *pstsiMask );
		}
	}

	// リストビューに追加

	for( size_t i = 0 ; i < vcFileInfo.size() ; i++ )
	{
		pclArc->AddFileInfo( vcFileInfo[i] );
	}

	for( size_t i = 0 ; i < vcNotMaskFileInfo.size() ; i++ )
	{
		pclArc->AddFileInfo( vcNotMaskFileInfo[i] );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード

BOOL	CWill::Decode(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( (pstFileInfo->format != _T("WIP")) && (pstFileInfo->format != _T("MSK")) )
	{
		return	FALSE;
	}

	// データの読み込み

	DWORD				dwSrcSize = pstFileInfo->sizeCmp;

	YCMemory<BYTE>		clmbtSrc( dwSrcSize );
	DWORD				dwSrcPtr = 0;

	pclArc->Read( &clmbtSrc[0], dwSrcSize );

	// ファイル数、色数、取得

	WORD				wFiles = *(WORD*) &clmbtSrc[4];
	WORD				wBpp = *(WORD*) &clmbtSrc[6];

	dwSrcPtr += 8;

	// 幅、高さ、取得

	std::vector<long>	vclWidth;
	std::vector<long>	vclHeight;
	std::vector<DWORD>	vcdwSrcSize;

	for( WORD i = 0 ; i < wFiles ; i++ )
	{
		vclWidth.push_back( *(long*) &clmbtSrc[dwSrcPtr + 0] );
		vclHeight.push_back( *(long*) &clmbtSrc[dwSrcPtr + 4] );
		vcdwSrcSize.push_back( *(DWORD*) &clmbtSrc[dwSrcPtr + 20] );

		dwSrcPtr += 24;
	}

	// マスク画像の有無

	BOOL				bExistsMask = !pstFileInfo->starts.empty();

	// マスク画像の取得

	DWORD				dwSrcSizeForMask;
	YCMemory<BYTE>		clmbtSrcForMask;
	DWORD				dwSrcPtrForMask;
	WORD				wFilesForMask;
	WORD				wBppForMask;
	std::vector<long>	vclWidthForMask;
	std::vector<long>	vclHeightForMask;
	std::vector<DWORD>	vcdwSrcSizeForMask;

	if( bExistsMask )
	{
		// マスク画像が存在する

		dwSrcSizeForMask = pstFileInfo->sizesCmp[0];
		dwSrcPtrForMask = 0;

		clmbtSrcForMask.resize( dwSrcSizeForMask );

		// マスク画像の読み込み

		pclArc->SeekHed( pstFileInfo->starts[0] );
		pclArc->Read( &clmbtSrcForMask[0], dwSrcSizeForMask );

		// ファイル数、色数、取得

		wFilesForMask = *(WORD*) &clmbtSrcForMask[4];
		wBppForMask = *(WORD*) &clmbtSrcForMask[6];

		dwSrcPtrForMask += 8;

		// 幅、高さ、取得

		for( WORD i = 0 ; i < wFilesForMask ; i++ )
		{
			vclWidthForMask.push_back( *(long*) &clmbtSrcForMask[dwSrcPtrForMask + 0] );
			vclHeightForMask.push_back( *(long*) &clmbtSrcForMask[dwSrcPtrForMask + 4] );
			vcdwSrcSizeForMask.push_back( *(DWORD*) &clmbtSrcForMask[dwSrcPtrForMask + 20] );

			dwSrcPtrForMask += 24;
		}

		// ファイル数が同じか確認

		bExistsMask = (wFiles == wFilesForMask);
	}

	// 出力

	for( WORD i = 0 ; i < wFiles ; i++ )
	{
		// マスク画像の付加で変更されるため再取得

		WORD				wBpp = *(WORD*) &clmbtSrc[6];

		// 出力用バッファ確保

		DWORD				dwDstSize = vclWidth[i] * vclHeight[i] * (wBpp >> 3);

		YCMemory<BYTE>		clmbtDst( dwDstSize );

		ZeroMemory( &clmbtDst[0], dwDstSize );

		BYTE*				pbtDst = &clmbtDst[0];

		// パレットの取得

		BYTE*				pbtPallet = NULL;

		if( wBpp == 8 )
		{
			pbtPallet = &clmbtSrc[dwSrcPtr];

			dwSrcPtr += 1024;
		}

		// LZSS解凍

		DecompLZSS( &clmbtDst[0], dwDstSize, &clmbtSrc[dwSrcPtr], vcdwSrcSize[i] );

		dwSrcPtr += vcdwSrcSize[i];

		// マスク画像を付加して32bit化

		DWORD				dwDstSizeFor32bit;

		YCMemory<BYTE>		clmbtDstFor32bit;

		if( bExistsMask )
		{
			// マスク画像が存在する

			DWORD				dwDstSizeForMask = vclWidthForMask[i] * vclHeightForMask[i] * (wBppForMask >> 3);

			YCMemory<BYTE>		clmbtDstForMask( dwDstSizeForMask );

			ZeroMemory( &clmbtDstForMask[0], dwDstSizeForMask );

			// パレットの取得

			BYTE*				pbtPalletForMask = NULL;

			if( wBppForMask == 8 )
			{
				pbtPalletForMask = &clmbtSrcForMask[dwSrcPtrForMask];

				dwSrcPtrForMask += 1024;
			}

			// LZSS解凍

			DecompLZSS( &clmbtDstForMask[0], dwDstSizeForMask, &clmbtSrcForMask[dwSrcPtrForMask], vcdwSrcSizeForMask[i] );

			dwSrcPtrForMask += vcdwSrcSizeForMask[i];

			// マスク画像の付加

			dwDstSizeFor32bit = vclWidth[i] * vclHeight[i] * 4;

			clmbtDstFor32bit.resize( dwDstSizeFor32bit );

			if( AppendMask( &clmbtDstFor32bit[0], dwDstSizeFor32bit, &clmbtDst[0], dwDstSize, &clmbtDstForMask[0], dwDstSizeForMask ) )
			{
				// マスク画像の付加に成功

				wBpp = 32;
				dwDstSize = dwDstSizeFor32bit;
				pbtDst = &clmbtDstFor32bit[0];
			}
		}

		// ファイル名の取得

		TCHAR				szFileExt[256];

		if( wFiles == 1 )
		{
			// ファイルが1個

			lstrcpy( szFileExt, _T("") );
		}
		else
		{
			// ファイルが2個以上

			_stprintf( szFileExt, _T("_%03d.bmp"), i );
		}

		// プログレスバー進捗要求

		BOOL				bProgress = TRUE;

		if( i >= 1 )
		{
			bProgress = FALSE;
		}

		// 出力

		CImage				clImage;

		clImage.Init( pclArc, vclWidth[i], vclHeight[i], wBpp, pbtPallet, 1024, szFileExt );
		clImage.WriteCompoBGRAReverse( pbtDst, dwDstSize, bProgress );
		clImage.Close();
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	LZSSの解凍

void	CWill::DecompLZSS(
	void*				pvDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const void*			pvSrc,							// 圧縮データ
	DWORD				dwSrcSize						// 圧縮データサイズ
	)
{
	const BYTE*			pbtSrc = (const BYTE*) pvSrc;
	BYTE*				pbtDst = (BYTE*) pvDst;

	DWORD				dwSrcPtr = 0;
	DWORD				dwDstPtr = 0;

	// スライド辞書

	DWORD				dwDicSize = 4096;

	YCMemory<BYTE>		clmbtDic( dwDicSize );

	ZeroMemory( &clmbtDic[0], dwDicSize );

	DWORD				dwDicPtr = 1;

	while( (dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) )
	{
		BYTE				btFlags = pbtSrc[dwSrcPtr++];

		for( DWORD i = 0 ; (i < 8) && (dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) ; i++ )
		{
			if( btFlags & 1 )
			{
				// 無圧縮データ

				pbtDst[dwDstPtr++] = clmbtDic[dwDicPtr++] = pbtSrc[dwSrcPtr++];

				dwDicPtr &= (dwDicSize - 1);
			}
			else
			{
				// 圧縮データ

				BYTE				btLow = pbtSrc[dwSrcPtr++];
				BYTE				btHigh = pbtSrc[dwSrcPtr++];

				DWORD				dwBack = ((btLow << 8) | btHigh) >> 4;

				if( dwBack == 0 )
				{
					// 解凍完了

					return;
				}

				// 辞書から読み込む長さ取得

				DWORD				dwLength = (btHigh & 0x0F) + 2;

				if( (dwDstPtr + dwLength) > dwDstSize )
				{
					// 出力バッファを超えてしまう

					dwLength = (dwDstSize - dwDstPtr);
				}

				// 辞書のデータを入力

				for( DWORD j = 0 ; j < dwLength ; j++ )
				{
					pbtDst[dwDstPtr++] = clmbtDic[dwDicPtr++] = clmbtDic[dwBack++];

					dwDicPtr &= (dwDicSize - 1);
					dwBack &= (dwDicSize - 1);
				}
			}

			btFlags >>= 1;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	マスク画像を付加して32bit化する

BOOL	CWill::AppendMask(
	BYTE*				pbtDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const BYTE*			pbtSrc,							// 24bitデータ
	DWORD				dwSrcSize,						// 24bitデータサイズ
	const BYTE*			pbtMask,						// 8bitデータ(マスク)
	DWORD				dwMaskSize						// 8bitデータサイズ
	)
{
	// 合成

	memcpy( pbtDst, pbtSrc, dwSrcSize );

	memcpy( &pbtDst[dwSrcSize], pbtMask, dwMaskSize );

	return	TRUE;
}
