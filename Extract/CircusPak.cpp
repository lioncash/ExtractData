
#include	"stdafx.h"
#include	"../ExtractBase.h"
#include	"../Image.h"
#include	"../Tga.h"
#include	"../Arc/LZSS.h"
#include	"CircusPak.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CCircusPak::Mount(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( MountPakForKujiraCons( pclArc) )
	{
		return	TRUE;
	}

	if( MountPakForKujira( pclArc ) )
	{
		return	TRUE;
	}

	if( MountPakForACDC( pclArc ) )
	{
		return	TRUE;
	}

	if( MountPakForDCGS( pclArc ) )
	{
		return	TRUE;
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	最終試験くじら-Conservative-向けpakのマウント

BOOL	CCircusPak::MountPakForKujiraCons(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".pak") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "File Pack 1.0y", 14 ) != 0 )
	{
		return	FALSE;
	}

	// ファイル数の取得

	DWORD				dwFiles;

	pclArc->SeekHed( 16 );
	pclArc->Read( &dwFiles, 4 );

	// 復号

	dwFiles ^= 0xAAAAAAAA;

	// インデックス取得

	YCMemory<SPakFileInfoType1>	clmIndex( dwFiles );

	pclArc->SeekCur( 4 );
	pclArc->Read( &clmIndex[0], (sizeof(SPakFileInfoType1) * dwFiles) );

	// ファイル情報の取得

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// ファイル名取得

		char				szFileName[25];

		for( DWORD j = 0 ; j < 24 ; j++ )
		{
			szFileName[j] = clmIndex[i].szFileName[j] ^ 0xAA;
		}

		szFileName[24] = '\0';

		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.start = clmIndex[i].dwOffset ^ 0xAAAAAAAA;
		stFileInfo.sizeCmp = clmIndex[i].dwCompFileSize ^ 0xAAAAAAAA;
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	最終試験くじら向けpakのマウント

BOOL	CCircusPak::MountPakForKujira(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".pak") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "File Pack 1.00", 14 ) != 0 )
	{
		return	FALSE;
	}

	// ファイル数の取得

	DWORD				dwFiles;

	pclArc->SeekHed( 16 );
	pclArc->Read( &dwFiles, 4 );

	// 復号

	dwFiles ^= 0xAAAAAAAA;

	// インデックス取得

	YCMemory<SPakFileInfoType2>	clmIndex( dwFiles );

	pclArc->SeekCur( 4 );
	pclArc->Read( &clmIndex[0], (sizeof(SPakFileInfoType2) * dwFiles) );

	// ファイル情報の取得

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// ファイル名取得

		char				szFileName[33];

		for( DWORD j = 0 ; j < 32 ; j++ )
		{
			szFileName[j] = clmIndex[i].szFileName[j] ^ 0xAA;
		}

		szFileName[32] = '\0';

		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.start = clmIndex[i].dwOffset ^ 0xAAAAAAAA;
		stFileInfo.sizeCmp = clmIndex[i].dwCompFileSize ^ 0xAAAAAAAA;
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ACDC向けpakのマウント

BOOL	CCircusPak::MountPakForACDC(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".pak") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "ACDC_RA", 7 ) != 0 )
	{
		return	FALSE;
	}

	// ファイル数の取得

	DWORD				dwFiles;

	pclArc->SeekHed( 8 );
	pclArc->Read( &dwFiles, 4 );

	// 復号

	dwFiles ^= 0xA6A6A6A6;

	// インデックス取得

	YCMemory<SPakFileInfoType2>	clmIndex( dwFiles );

	pclArc->SeekCur( 4 );
	pclArc->Read( &clmIndex[0], (sizeof(SPakFileInfoType2) * dwFiles) );

	// ファイル情報の取得

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// ファイル名取得

		char				szFileName[33];

		for( DWORD j = 0 ; j < 32 ; j++ )
		{
			szFileName[j] = clmIndex[i].szFileName[j] ^ 0xA6;
		}

		szFileName[32] = '\0';

		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.start = clmIndex[i].dwOffset ^ 0xA6A6A6A6;
		stFileInfo.sizeCmp = clmIndex[i].dwCompFileSize ^ 0xA6A6A6A6;
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	DCGS向けpakのマウント

BOOL	CCircusPak::MountPakForDCGS(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".pak") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "PACK", 4 ) != 0 )
	{
		return	FALSE;
	}

	pclArc->SeekHed( 4 );

	// ファイル数の取得

	DWORD				dwFiles;

	pclArc->Read( &dwFiles, 4 );

	// フラグの取得

	DWORD				dwFlags;

	pclArc->Read( &dwFlags, 4 );

	// インデックスサイズの取得

	DWORD				dwIndexSize = (sizeof(SPakFileInfoType3) * dwFiles);

	// インデックスの取得

	YCMemory<SPakFileInfoType3>	clmIndex( dwFiles );

	pclArc->Read( &clmIndex[0], dwIndexSize );

	// インデックスの復号

	BYTE*				pbtIndex = (BYTE*) &clmIndex[0];

	for( DWORD i = 0 ; i < dwIndexSize ; i++ )
	{
		pbtIndex[i] = (pbtIndex[i] << 4) | (pbtIndex[i] >> 4);
	}

	// ファイル情報の取得

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name.Copy( clmIndex[i].szFileName, sizeof(clmIndex[i].szFileName) );
		stFileInfo.start = clmIndex[i].dwOffset;
		stFileInfo.sizeCmp = clmIndex[i].dwCompFileSize;
		stFileInfo.sizeOrg = clmIndex[i].dwFileSize;

		if( stFileInfo.sizeCmp == 0 )
		{
			stFileInfo.sizeCmp = stFileInfo.sizeOrg;
		}

		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード

BOOL	CCircusPak::Decode(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( DecodePakForKujiraCons( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodePakForKujira( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodePakForACDC( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodePakForDCGS( pclArc ) )
	{
		return	TRUE;
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	最終試験くじら-Conservative-向けpakのデコード

BOOL	CCircusPak::DecodePakForKujiraCons(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pclArc->GetArcExten() != _T(".pak") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "File Pack 1.0y", 14 ) != 0 )
	{
		return	FALSE;
	}

	BYTE				abtKey[2];

	abtKey[0] = 0xFF;
	abtKey[1] = (pstFileInfo->format == _T("CS")) ? 1 : 0;

	if( pstFileInfo->format == _T("BMP") )
	{
		// BMP

		DWORD				dwSrcSize = pstFileInfo->sizeCmp;

		YCMemory<BYTE>		clmbtSrc( dwSrcSize );

		pclArc->Read( &clmbtSrc[0], dwSrcSize );

		// 復号

		Decrypt1( &clmbtSrc[0], dwSrcSize, abtKey );

		// 出力

		DecodeBMP( pclArc, &clmbtSrc[0], dwSrcSize );
	}
	else if( pstFileInfo->format == _T("CPS") )
	{
		// CPS

		DWORD				dwSrcSize = pstFileInfo->sizeCmp;

		YCMemory<BYTE>		clmbtSrc( dwSrcSize );

		pclArc->Read( &clmbtSrc[0], dwSrcSize );

		// 復号

		Decrypt1( &clmbtSrc[0], dwSrcSize, abtKey );

		*(DWORD*) &clmbtSrc[4] ^= 0x80701084;

		// デコード

		DecodeCps( pclArc, &clmbtSrc[0], dwSrcSize );
	}
	else
	{
		// その他

		DecodeEtc( pclArc, Decrypt2, abtKey );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	最終試験くじら向けpakのデコード

BOOL	CCircusPak::DecodePakForKujira(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pclArc->GetArcExten() != _T(".pak") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "File Pack 1.00", 14 ) != 0 )
	{
		return	FALSE;
	}

	BYTE				abtKey[2];

	abtKey[0] = 0xFF;
	abtKey[1] = (pstFileInfo->format == _T("CS")) ? 1 : 0;

	if( pstFileInfo->format == _T("BMP") )
	{
		// 読み込み

		DWORD				dwSrcSize = pstFileInfo->sizeCmp;

		YCMemory<BYTE>		clmbtSrc( dwSrcSize );

		pclArc->Read( &clmbtSrc[0], dwSrcSize );

		// 復号

		Decrypt1( &clmbtSrc[0], dwSrcSize, abtKey );

		// 出力

		DecodeBMP( pclArc, &clmbtSrc[0], dwSrcSize );
	}
	else if( pstFileInfo->format == _T("CPS") )
	{
		// CPS

		DWORD				dwSrcSize = pstFileInfo->sizeCmp;

		YCMemory<BYTE>		clmbtSrc( dwSrcSize );

		pclArc->Read( &clmbtSrc[0], dwSrcSize );

		// 復号

		Decrypt1( &clmbtSrc[0], dwSrcSize, abtKey );

		*(DWORD*) &clmbtSrc[4] ^= 0x80701084;

		// デコード

		DecodeCps( pclArc, &clmbtSrc[0], dwSrcSize );
	}
	else
	{
		// その他

		DecodeEtc( pclArc, Decrypt2, abtKey );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ACDC向けpakのデコード

BOOL	CCircusPak::DecodePakForACDC(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pclArc->GetArcExten() != _T(".pak") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "ACDC_RA", 7 ) != 0 )
	{
		return	FALSE;
	}

	BYTE				abtKey[2];

	abtKey[0] = 0x68;
	abtKey[1] = (pstFileInfo->format == _T("CS")) ? 1 : 0;

	if( pstFileInfo->format == _T("BMP") )
	{
		// BMP

		DWORD				dwSrcSize = pstFileInfo->sizeCmp;

		YCMemory<BYTE>		clmbtSrc( dwSrcSize );

		pclArc->Read( &clmbtSrc[0], dwSrcSize );

		// 復号

		Decrypt1( &clmbtSrc[0], dwSrcSize, abtKey );

		// 出力

		DecodeBMP( pclArc, &clmbtSrc[0], dwSrcSize );
	}
	else if( pstFileInfo->format == _T("TGA") )
	{
		// TGA

		DWORD				dwSrcSize = pstFileInfo->sizeCmp;

		YCMemory<BYTE>		clmbtSrc( dwSrcSize );

		pclArc->Read( &clmbtSrc[0], dwSrcSize );

		// 復号

		Decrypt1( &clmbtSrc[0], dwSrcSize, abtKey );

		// 出力

		CTga				clTGA;

		clTGA.Decode( pclArc, &clmbtSrc[0], dwSrcSize );
	}
	else if( pstFileInfo->format == _T("CPS") )
	{
		// CPS

		DWORD				dwSrcSize = pstFileInfo->sizeCmp;

		YCMemory<BYTE>		clmbtSrc( dwSrcSize );

		pclArc->Read( &clmbtSrc[0], dwSrcSize );

		// 復号

		Decrypt1( &clmbtSrc[0], dwSrcSize, abtKey );

		*(DWORD*) &clmbtSrc[4] ^= 0x0A415FCF;

		if( dwSrcSize >= 0x308 )
		{
			*(DWORD*) &clmbtSrc[4] ^= clmbtSrc[dwSrcSize - 1];

			DWORD				dwWork;
			DWORD				dwWork2;

			// データの入れ替え

			dwWork = 8 + (dwSrcSize - 8) - 0xFF;
			dwWork2 = 8 + ((dwSrcSize - 8) >> 9) + 0xFF;

			std::swap( clmbtSrc[dwWork], clmbtSrc[dwWork2] );

			// データの入れ替え

			dwWork = 8 + (dwSrcSize - 8) - (0xFF << 1);
			dwWork2 = 8 + (((dwSrcSize - 8) >> 9) << 1) + 0xFF;

			std::swap( clmbtSrc[dwWork], clmbtSrc[dwWork2] );
		}

		// デコード

		DecodeCps( pclArc, &clmbtSrc[0], dwSrcSize );
	}
	else
	{
		// その他

		DecodeEtc( pclArc, Decrypt2, abtKey );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	DCGS向けpakのデコード

BOOL	CCircusPak::DecodePakForDCGS(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pclArc->GetArcExten() != _T(".pak") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "PACK", 4 ) != 0 )
	{
		return	FALSE;
	}

	// 読み込み

	DWORD				dwSrcSize = pstFileInfo->sizeCmp;

	YCMemory<BYTE>		clmSrc( dwSrcSize );

	pclArc->Read( &clmSrc[0], dwSrcSize );

	// 解凍

	BYTE*				pbtDst = &clmSrc[0];
	DWORD				dwDstSize = pstFileInfo->sizeOrg;
	YCMemory<BYTE>		clmDst;

	if( dwSrcSize != dwDstSize )
	{
		// 圧縮されている

		clmDst.resize( dwDstSize );

		// 解凍

		CLZSS				clLZSS;

		clLZSS.Decomp( &clmDst[0], dwDstSize, &clmSrc[0], dwSrcSize, 4096, 4078, 3 );

		pbtDst = &clmDst[0];
	}

	// 出力

	if( pstFileInfo->format == _T("OP2") )
	{
		// 画像

		long				lWidth = *(long*) &pbtDst[4];
		long				lHeight = *(long*) &pbtDst[8];
		WORD				wBpp = *(WORD*) &pbtDst[12];
		DWORD				dwDataOffset = *(DWORD*) &pbtDst[20];
		DWORD				dwPalletSize = (dwDataOffset - 32);
		DWORD				dwDIBSize = *(DWORD*) &pbtDst[24];

		dwDstSize = *(DWORD*) &pbtDst[28];

		YCMemory<BYTE>		clmDIB( dwDIBSize );

		// 解凍

		CLZSS				clLZSS;

		clLZSS.Decomp( &clmDIB[0], dwDIBSize, &pbtDst[dwDataOffset], dwDstSize, 4096, 4078, 3 );

		// 出力

		CImage				clImage;

		clImage.Init( pclArc, lWidth, lHeight, wBpp, &pbtDst[32], dwPalletSize );
		clImage.WriteReverse( &clmDIB[0], dwDIBSize );
		clImage.Close();
	}
	else
	{
		// その他

		pclArc->OpenFile();
		pclArc->WriteFile( pbtDst, dwDstSize );
		pclArc->CloseFile();
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	BMPのデコード

BOOL	CCircusPak::DecodeBMP(
	CArcFile*			pclArc,							// アーカイブ
	const void*			pvSrc,							// BMPデータ(復号化済みであること)
	DWORD				dwSrcSize						// BMPデータサイズ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->format != _T("BMP") )
	{
		return	FALSE;
	}

	BYTE*				pbtSrc = (BYTE*) pvSrc;

	BITMAPFILEHEADER*	pstbmfhSrc = (BITMAPFILEHEADER*) &pbtSrc[0];
	BITMAPINFOHEADER*	pstbmihSrc = (BITMAPINFOHEADER*) &pbtSrc[14];

	dwSrcSize = (pstbmihSrc->biBitCount == 8) ? (dwSrcSize - 54 - 1024) : (dwSrcSize - 54);

	CImage				clImage;

	clImage.Init( pclArc, pbtSrc );
	clImage.Write( dwSrcSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	CPSのデコード

BOOL	CCircusPak::DecodeCps(
	CArcFile*			pclArc,							// アーカイブ
	const void*			pvSrc,							// CPSデータ(復号化済みであること)
	DWORD				dwSrcSize						// CPSデータサイズ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->format != _T("CPS") )
	{
		return	FALSE;
	}

	const BYTE*			pbtSrc = (const BYTE*) pvSrc;

	// 出力バッファ確保

	DWORD				dwDstSize = *(DWORD*) &pbtSrc[4];

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// 解凍

	if( memcmp( pbtSrc, "CCC0", 4 ) == 0 )
	{
		// CCC0

		DecompCCC0( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize );

		long				lWidth = *(WORD*) &clmbtDst[12];
		long				lHeight = *(WORD*) &clmbtDst[14];
		WORD				wBpp = clmbtDst[16];

		// 出力

		CImage				clImage;

		clImage.Init( pclArc, lWidth, lHeight, wBpp );
		clImage.Write( &clmbtDst[18], (dwDstSize - 18) );
	}
	else if( memcmp( pbtSrc, "CCM0", 4 ) == 0 )
	{
		// CCM0

		DecompCCM0( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize );

		// 出力

		BITMAPFILEHEADER*	pstBMPFileHeader = (BITMAPFILEHEADER*) &clmbtDst[0];
		BITMAPINFOHEADER*	pstBMPInfoHeader = (BITMAPINFOHEADER*) &clmbtDst[14];

		dwDstSize = (pstBMPInfoHeader->biBitCount == 8) ? (dwDstSize - 54 - 1024) : (dwDstSize - 54);

		CImage				clImage;

		clImage.Init( pclArc, &clmbtDst[0] );
		clImage.Write( dwDstSize );
	}
	else
	{
		// 不明

		pclArc->OpenFile();
		pclArc->WriteFile( pbtSrc, dwSrcSize );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	その他のデータのデコード

BOOL	CCircusPak::DecodeEtc(
	CArcFile*			pclArc,							// アーカイブ
	FDecrypt			pfnDecryptFunc,					// 復号関数へのポインタ
	const void*			pvKey							// 復号キー
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();
	DWORD				dwBufSize = pclArc->GetBufSize();

	YCMemory<BYTE>		clmbtBuf( dwBufSize );

	// 出力ファイル生成

	if( pstFileInfo->format == _T("CS") )
	{
		pclArc->OpenScriptFile();
	}
	else
	{
		pclArc->OpenFile();
	}

	for( DWORD dwWriteSize = 0 ; dwWriteSize < pstFileInfo->sizeOrg ; dwWriteSize += dwBufSize )
	{
		// バッファサイズ調整

		pclArc->SetBufSize( &dwBufSize, dwWriteSize );

		// 復号したデータを出力

		pclArc->Read( &clmbtBuf[0], dwBufSize );

		pfnDecryptFunc( &clmbtBuf[0], dwBufSize, pvKey );

		pclArc->WriteFile( &clmbtBuf[0], dwBufSize );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	CCC0の解凍

BOOL	CCircusPak::DecompCCC0(
	void*				pvDst,							// 格納先
	DWORD				dwDstSize,						// 格納先のサイズ
	const void*			pvSrc,							// CCC0データ
	DWORD				dwSrcSize						// CCC0データサイズ
	)
{
	const BYTE*			pbtSrc = (const BYTE*) pvSrc;
	BYTE*				pbtDst = (BYTE*) pvDst;

	DWORD				dwSrcPtrOfBit = 0x2C * 8;
	DWORD				dwDstPtr = 0;

	// 解凍

	for( dwDstPtr = 0 ; dwDstPtr < dwDstSize ; dwDstPtr += 2 )
	{
		DWORD			dwIndex;

		for( dwIndex = 0 ; dwIndex < 0x0F ; dwIndex++ )
		{
			if( GetBit( pbtSrc, &dwSrcPtrOfBit, 1 ) == 1 )
			{
				break;
			}
		}

		if( dwIndex == 0x0F )
		{
			if( GetBit( pbtSrc, &dwSrcPtrOfBit, 1 ) == 0 )
			{
				dwIndex++;
			}
		}

		if( dwIndex != 0 )
		{
			*(WORD*) &pbtDst[dwDstPtr] = ((WORD*) (pbtSrc + 8))[dwIndex - 1];
		}
		else
		{
			*(WORD*) &pbtDst[dwDstPtr] = GetBit( pbtSrc, &dwSrcPtrOfBit, 16 );
		}
	}

	// サイズが奇数の場合1余る

	if( dwDstSize & 1 )
	{
		pbtDst[dwDstPtr] = pbtSrc[0x28];
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	CCM0の解凍

BOOL	CCircusPak::DecompCCM0(
	void*				pvDst,							// 格納先
	DWORD				dwDstSize,						// 格納先のサイズ
	const void*			pvSrc,							// CCM0データ
	DWORD				dwSrcSize						// CCM0データサイズ
	)
{
	const BYTE*			pbtSrc = (const BYTE*) pvSrc;
	BYTE*				pbtDst = (BYTE*) pvDst;

	DWORD				dwSrcPtrOfBit;
	DWORD				dwDstPtr = 0;

	// 解凍

	if( dwDstSize < 0x80 )
	{
		// 無圧縮

		memcpy( pbtDst, &pbtSrc[8], dwDstSize );
	}
	else
	{
		for( DWORD i = 0 ; i < 0x80 ; i++ )
		{
			pbtDst[dwDstPtr++] = pbtSrc[8 + i] + 0xF0;
		}

		dwSrcPtrOfBit = 0x88 * 8;

		while( (dwDstPtr < dwDstSize) )
		{
			DWORD				dwFlag = GetBit( pbtSrc, &dwSrcPtrOfBit, 1 );

			if( dwFlag & 1 )
			{
				// 圧縮

				DWORD				dwBack = GetBit( pbtSrc, &dwSrcPtrOfBit, 7 ) + 1;
				DWORD				dwLength = GetBit( pbtSrc, &dwSrcPtrOfBit, 4 ) + 2;

				for( DWORD i = 0 ; i < dwLength ; i++ )
				{
					pbtDst[dwDstPtr + i] = pbtDst[dwDstPtr - dwBack + i];
				}

				dwDstPtr += dwLength;
			}
			else
			{
				// 無圧縮

				pbtDst[dwDstPtr++] = GetBit( pbtSrc, &dwSrcPtrOfBit, 8 );
			}
		}
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ビット列の取得

DWORD	CCircusPak::GetBit(
	const void*			pvSrc,							// 入力データ
	DWORD*				pdwSrcPtrOfBit,					// 入力データの位置(読み込んだビット数が加算される)
	DWORD				dwReadBitLength					// 読み込むビット数
	)
{
	const BYTE*			pbtSrc = (const BYTE*) pvSrc;
	DWORD				dwData = 0;

	for( DWORD i = 0 ; i < dwReadBitLength ; i++ )
	{
		dwData <<= 1;
		dwData |= (pbtSrc[*pdwSrcPtrOfBit >> 3] >> (*pdwSrcPtrOfBit & 7)) & 1;
		(*pdwSrcPtrOfBit)++;
	}

	return	dwData;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号1

BOOL	CCircusPak::Decrypt1(
	void*				pvTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	const void*			pvKey							// 復号値
	)
{
	BYTE*				pbtTarget = (BYTE*) pvTarget;
	const BYTE*			pbtKey = (const BYTE*) pvKey;

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= pbtKey[0];
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号2

BOOL	CCircusPak::Decrypt2(
	void*				pvTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	const void*			pvKey							// 復号値
	)
{
	BYTE*				pbtTarget = (BYTE*) pvTarget;
	const BYTE*			pbtKey = (const BYTE*) pvKey;

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= pbtKey[0];
		pbtTarget[i] -= pbtKey[1];
	}

	return	TRUE;
}
