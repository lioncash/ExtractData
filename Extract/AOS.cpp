
#include	"stdafx.h"
#include	"../Image.h"
#include	"AOS.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CAOS::Mount(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".aos") )
	{
		return	FALSE;
	}

	// 不明な4バイト

	pclArc->SeekHed( 4 );

	// オフセットの取得

	DWORD				dwOffset;

	pclArc->Read( &dwOffset, 4 );

	// インデックスサイズの取得

	DWORD				dwIndexSize;

	pclArc->Read( &dwIndexSize, 4 );

	// アーカイブファイル名の取得

	char				szArchiveName[261];

	pclArc->Read( szArchiveName, 261 );

	if( pclArc->GetArcName() != szArchiveName )
	{
		// アーカイブファイル名が異なる

		pclArc->SeekHed();
		return	FALSE;
	}

	// インデックスの取得

	YCMemory<BYTE>		clmIndex( dwIndexSize );

	pclArc->Read( &clmIndex[0], dwIndexSize );

	// ファイル情報の取得

	for( DWORD i = 0 ; i < dwIndexSize ; i += 40 )
	{
		SFileInfo			stFileInfo;

		stFileInfo.name.Copy( (char*) &clmIndex[i], 32 );
		stFileInfo.start = *(DWORD*) &clmIndex[i + 32] + dwOffset;
		stFileInfo.sizeCmp = *(DWORD*) &clmIndex[i + 36];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード

BOOL	CAOS::Decode(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".aos") )
	{
		return	FALSE;
	}

	BOOL				bReturn = FALSE;
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->format == _T("ABM") )
	{
		// ABM

		bReturn = DecodeABM( pclArc );
	}
	else if( pstFileInfo->format == _T("MSK") )
	{
		// マスク画像

		bReturn = DecodeMask( pclArc );
	}
	else if( pstFileInfo->format == _T("SCR") )
	{
		// スクリプト

		bReturn = DecodeScript( pclArc );
	}

	return	bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ABMのデコード

BOOL	CAOS::DecodeABM(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	// データの読み込み

	DWORD				dwSrcSize = pstFileInfo->sizeCmp;

	YCMemory<BYTE>		clmSrc( dwSrcSize );

	pclArc->Read( &clmSrc[0], dwSrcSize );

	// ビットマップヘッダの取得

	BITMAPFILEHEADER*	pstbfhSrc = (BITMAPFILEHEADER*) &clmSrc[0];
	BITMAPINFOHEADER*	pstbihSrc = (BITMAPINFOHEADER*) &clmSrc[14];

	CImage				clImage;
	YCMemory<BYTE>		clmDst;
	YCString			clsLastName;
	DWORD				dwDstSize;
	DWORD				dwFrames;
	DWORD				dwOffsetToData;
	DWORD				dwSrcPtr = 0;
	DWORD				dwDstPtr = 0;

	switch( pstbihSrc->biBitCount )
	{
	case	1:
		// マルチフレーム

		dwFrames = *(DWORD*) &clmSrc[58];
		dwOffsetToData = *(DWORD*) &clmSrc[66];

		dwDstSize = (pstbihSrc->biWidth * pstbihSrc->biHeight * 4);

		clmDst.resize( dwDstSize );

		if( dwFrames >= 2 )
		{
			// 複数ファイル

			clsLastName.Format( _T("_000") );
		}

		// 解凍

		dwSrcPtr = dwOffsetToData;

		for( DWORD i = 0 ; i < dwDstSize ; i += 4 )
		{
			clmDst[i + 0] = clmSrc[dwSrcPtr++];
			clmDst[i + 1] = clmSrc[dwSrcPtr++];
			clmDst[i + 2] = clmSrc[dwSrcPtr++];
			clmDst[i + 3] = 0xFF;
		}

		// 出力

		clImage.Init( pclArc, pstbihSrc->biWidth, pstbihSrc->biHeight, 32, NULL, 0, clsLastName );
		clImage.WriteReverse( &clmDst[0], dwDstSize );
		clImage.Close();

		// 

		for( DWORD i = 1 ; i < dwFrames ; i++ )
		{
			DWORD				dwOffsetToFrame = *(DWORD*) &clmSrc[70 + (i - 1) * 4];

			clsLastName.Format( _T("_%03d"), i );

			// 解凍

			ZeroMemory( &clmDst[0], dwDstSize );

			DecompABM( &clmDst[0], dwDstSize, &clmSrc[dwOffsetToFrame], (dwSrcSize - dwOffsetToFrame) );

			// 出力

			clImage.Init( pclArc, pstbihSrc->biWidth, pstbihSrc->biHeight, 32, NULL, 0, clsLastName );
			clImage.WriteReverse( &clmDst[0], dwDstSize, FALSE );
			clImage.Close();
		}

		break;

	case	32:
		// 32bit

		dwDstSize = (pstbihSrc->biWidth * pstbihSrc->biHeight * 4);

		clmDst.resize( dwDstSize );

		// 解凍

		DecompABM( &clmDst[0], dwDstSize, &clmSrc[54], (dwSrcSize - 54) );

		// 出力

		clImage.Init( pclArc, pstbihSrc->biWidth, pstbihSrc->biHeight, pstbihSrc->biBitCount );
		clImage.WriteReverse( &clmDst[0], dwDstSize );
		clImage.Close();

		break;

	default:
		// その他

		pclArc->OpenFile();
		pclArc->WriteFile( &clmSrc[0], dwSrcSize );
		pclArc->CloseFile();
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	マスク画像のデコード

BOOL	CAOS::DecodeMask(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	// データの読み込み

	DWORD				dwSrcSize = pstFileInfo->sizeCmp;

	YCMemory<BYTE>		clmSrc( dwSrcSize );

	pclArc->Read( &clmSrc[0], dwSrcSize );

	// 出力

	CImage				clImage;

	clImage.Init( pclArc, &clmSrc[0] );
	clImage.Write( dwSrcSize );
	clImage.Close();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	スクリプトのデコード

BOOL	CAOS::DecodeScript(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	// 圧縮データの読み込み

	DWORD				dwSrcSize = pstFileInfo->sizeCmp;

	YCMemory<BYTE>		clmSrc( dwSrcSize );

	pclArc->Read( &clmSrc[0], dwSrcSize );

	// 解凍用バッファの確保

	DWORD				dwDstSize = *(DWORD*) &clmSrc[0];

	YCMemory<BYTE>		clmDst( dwDstSize );

	// 解凍

	DecompScript( &clmDst[0], dwDstSize, &clmSrc[4], (dwSrcSize - 4) );

	// 出力

	pclArc->OpenScriptFile();
	pclArc->WriteFile( &clmDst[0], dwDstSize, dwSrcSize );
	pclArc->CloseFile();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ABMの解凍

BOOL	CAOS::DecompABM(
	BYTE*				pbtDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize						// 圧縮データサイズ
	)
{
	DWORD				dwSrcPtr = 0;
	DWORD				dwDstPtr = 0;

	BYTE				btCurrentSrc;
	DWORD				dwAlphaCount = 0;

	while( dwDstPtr < dwDstSize )
	{
		DWORD				dwLength;

		btCurrentSrc = pbtSrc[dwSrcPtr++];

		switch( btCurrentSrc )
		{
		case	0:
			// 0x00である

			dwLength = pbtSrc[dwSrcPtr++];

			for( DWORD i = 0 ; i < dwLength ; i++ )
			{
				pbtDst[dwDstPtr++] = 0;

				dwAlphaCount++;

				if( dwAlphaCount == 3 )
				{
					pbtDst[dwDstPtr++] = 0;

					dwAlphaCount = 0;
				}
			}

			break;

		case	255:
			// 0xFFである

			dwLength = pbtSrc[dwSrcPtr++];

			for( DWORD i = 0 ; i < dwLength ; i++ )
			{
				pbtDst[dwDstPtr++] = pbtSrc[dwSrcPtr++];

				dwAlphaCount++;

				if( dwAlphaCount == 3 )
				{
					pbtDst[dwDstPtr++] = 0xFF;

					dwAlphaCount = 0;
				}
			}

			break;

		default:
			// その他

			pbtDst[dwDstPtr++] = pbtSrc[dwSrcPtr++];

			dwAlphaCount++;

			if( dwAlphaCount == 3 )
			{
				pbtDst[dwDstPtr++] = btCurrentSrc;

				dwAlphaCount = 0;
			}

		}
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	スクリプトの解凍

BOOL	CAOS::DecompScript(
	BYTE*				pbtDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize						// 圧縮データサイズ
	)
{
	// テーブルの構築

	DWORD				adwTableOfBit0[511];
	DWORD				adwTableOfBit1[511];
	DWORD				dwSrcPtr = 0;
	DWORD				dwTablePtr = 256;
	DWORD				dwCurrentSrc = 0;
	DWORD				dwBitShift = 0;

	ZeroMemory( adwTableOfBit0, sizeof(adwTableOfBit0) );
	ZeroMemory( adwTableOfBit1, sizeof(adwTableOfBit1) );

	dwTablePtr = CreateHuffmanTable( adwTableOfBit0, adwTableOfBit1, pbtSrc, &dwSrcPtr, &dwTablePtr, &dwCurrentSrc, &dwBitShift );

	// 解凍

	DecompHuffman( pbtDst, dwDstSize, adwTableOfBit0, adwTableOfBit1, &pbtSrc[dwSrcPtr], dwTablePtr, dwCurrentSrc, dwBitShift );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	テーブルの構築

DWORD	CAOS::CreateHuffmanTable(
	DWORD*				pdwTableOfBit0,					// bit0用テーブル
	DWORD*				pdwTableOfBit1,					// bit1用テーブル
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD*				pdwSrcPtr,						// 圧縮データ参照位置
	DWORD*				pdwTablePtr,					// テーブル参照位置
	DWORD*				pdwCurrentSrc,					// 現在のデータ
	DWORD*				pdwBitShift						// ビットシフト
	)
{
	DWORD				dwReturn = 0;
	DWORD				dwTablePtr;

	if( *pdwBitShift == 0 )
	{
		// 8bit読み切った

		*pdwCurrentSrc = pbtSrc[(*pdwSrcPtr)++];
		*pdwBitShift = 8;
	}

	(*pdwBitShift) -= 1;

	if( (*pdwCurrentSrc >> *pdwBitShift) & 1 )
	{
		// ビットが1

		dwTablePtr = *pdwTablePtr;

		(*pdwTablePtr) += 1;

		if( dwTablePtr < 511 )
		{
			pdwTableOfBit0[dwTablePtr] = CreateHuffmanTable( pdwTableOfBit0, pdwTableOfBit1, pbtSrc, pdwSrcPtr, pdwTablePtr, pdwCurrentSrc, pdwBitShift );
			pdwTableOfBit1[dwTablePtr] = CreateHuffmanTable( pdwTableOfBit0, pdwTableOfBit1, pbtSrc, pdwSrcPtr, pdwTablePtr, pdwCurrentSrc, pdwBitShift );

			dwReturn = dwTablePtr;
		}
	}
	else
	{
		// ビットが0

		DWORD				dwBitShiftTemp = 8;
		DWORD				dwResult = 0;

		while( dwBitShiftTemp > *pdwBitShift )
		{
			DWORD				dwWork = ((1 << *pdwBitShift) - 1) & *pdwCurrentSrc;

			dwBitShiftTemp -= *pdwBitShift;

			*pdwCurrentSrc = pbtSrc[(*pdwSrcPtr)++];

			dwResult |= (dwWork << dwBitShiftTemp);

			*pdwBitShift = 8;
		}

		(*pdwBitShift) -= dwBitShiftTemp;

		DWORD				dwMask = (1 << dwBitShiftTemp) - 1;

		dwReturn = ((*pdwCurrentSrc >> *pdwBitShift) & dwMask) | dwResult;
	}

	return	dwReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ハフマンの解凍

BOOL	CAOS::DecompHuffman(
	BYTE*				pbtDst,							// 出力先
	DWORD				dwDstSize,						// 出力先サイズ
	const DWORD*		pdwTableOfBit0,					// bit0用テーブル
	const DWORD*		pdwTableOfBit1,					// bit1用テーブル
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwRoot,							// テーブル参照位置
	DWORD				dwCurrentSrc,					// 現在のデータ
	DWORD				dwBitShift						// ビットシフト
	)
{
	if( dwDstSize <= 0 )
	{
		return	FALSE;
	}

	DWORD				dwSrcPtr = 0;
	DWORD				dwDstPtr = 0;

	while( dwDstPtr < dwDstSize )
	{
		DWORD				dwTablePtr = dwRoot;

		while( dwTablePtr >= 256 )
		{
			if( dwBitShift == 0 )
			{
				// 8bit読み切った

				dwCurrentSrc = pbtSrc[dwSrcPtr++];
				dwBitShift = 8;
			}

			dwBitShift -= 1;

			if( (dwCurrentSrc >> dwBitShift) & 1 )
			{
				// bitが1

				dwTablePtr = pdwTableOfBit1[dwTablePtr];
			}
			else
			{
				// bitが0

				dwTablePtr = pdwTableOfBit0[dwTablePtr];
			}
		}

		pbtDst[dwDstPtr++] = (BYTE) dwTablePtr;
	}

	return	TRUE;
}
