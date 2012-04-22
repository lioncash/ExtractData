
#include	"stdafx.h"
#include	"../Arc/LZSS.h"
#include	"../Image.h"
#include	"Retouch.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CRetouch::Mount(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( MountGYU( pclArc ) )
	{
		return	TRUE;
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	GYUのマウント

BOOL	CRetouch::MountGYU(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".gyu") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "GYU\x1A", 4 ) != 0 )
	{
		return	FALSE;
	}

	return	pclArc->Mount();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード

BOOL	CRetouch::Decode(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( DecodeGYU( pclArc ) )
	{
		return	TRUE;
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	GYUのデコード

BOOL	CRetouch::DecodeGYU(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->format != _T("GYU") )
	{
		return	FALSE;
	}

	// ヘッダの読み込み

	SGYUHeader			stGYUHeader;

	pclArc->Read( &stGYUHeader, sizeof(stGYUHeader) );

	if( memcmp( stGYUHeader.abtIdentifier, "GYU\x1A", 4 ) != 0 )
	{
		// 関係ないファイル

		pclArc->SeekCur( -(INT64)sizeof(stGYUHeader) );
		return	FALSE;
	}

	// パレットの読み込み

	BYTE				abtPallet[1024];
	DWORD				dwPalletSize = (stGYUHeader.dwPallets * 4);

	ZeroMemory( abtPallet, sizeof(abtPallet) );

	if( dwPalletSize > 0 )
	{
		// パレットが存在する

		BYTE				abtBlackPallet[1024];

		ZeroMemory( abtBlackPallet, sizeof(abtBlackPallet) );

		pclArc->Read( abtPallet, dwPalletSize );

		if( memcmp( abtPallet, abtBlackPallet, dwPalletSize ) == 0 )
		{
			// 全てのパレットがゼロ

			for( DWORD i = 0, j = 0 ; i < dwPalletSize ; i += 4, j++ )
			{
				abtPallet[i + 0] = (BYTE) j;
				abtPallet[i + 1] = (BYTE) j;
				abtPallet[i + 2] = (BYTE) j;
				abtPallet[i + 3] = 0;
			}
		}
	}

	// 読み込み

	DWORD				dwSrcSize = stGYUHeader.adwCompSize[0];

	YCMemory<BYTE>		clmbtSrc( dwSrcSize );

	pclArc->Read( &clmbtSrc[0], dwSrcSize );

	// 出力用バッファの確保

	DWORD				dwDstSize = (((stGYUHeader.lWidth * (stGYUHeader.dwBpp >> 3) + 3) & 0xFFFFFFFC) * stGYUHeader.lHeight );

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// GYUの復号

	DecryptGYU( &clmbtSrc[0], dwSrcSize, stGYUHeader.dwKey );

	// GYUの解凍

	DecompGYU( &clmbtDst[0], dwDstSize, &clmbtSrc[0], dwSrcSize );

	// 出力

	if( stGYUHeader.adwCompSize[1] != 0 )
	{
		// アルファ値が存在する

		// 読み込み

		DWORD				dwSrcSize2 = stGYUHeader.adwCompSize[1];

		YCMemory<BYTE>		clmbtSrc2( dwSrcSize2 );

		pclArc->Read( &clmbtSrc2[0], dwSrcSize2 );

		// 出力用バッファの確保

		DWORD				dwDstSize2 = (((stGYUHeader.lWidth * (stGYUHeader.dwBpp >> 3) + 3) & 0xFFFFFFFC) * stGYUHeader.lHeight);

		YCMemory<BYTE>		clmbtDst2( dwDstSize2 );

		// GYUの解凍

		DecompGYU( &clmbtDst2[0], dwDstSize2, &clmbtSrc2[0], dwSrcSize2 );

		// 出力用バッファの確保

		DWORD				dwDstSize32Bit = (stGYUHeader.lWidth * stGYUHeader.lHeight * 4);

		YCMemory<BYTE>		clmbtDst32Bit( dwDstSize32Bit );

		// アルファ値を付加して32bit化

		for( long i = 0, lY = 0 ; (i < dwDstSize32Bit) && (lY < stGYUHeader.lHeight) ; lY++ )
		{
			long				lPresentHeight = (((stGYUHeader.lWidth + 3) & 0xFFFFFFFC) * lY);

			for( long lX = 0 ; lX < stGYUHeader.lWidth ; lX++ )
			{
				clmbtDst32Bit[i++] = abtPallet[clmbtDst[lPresentHeight + lX] * 4 + 0];
				clmbtDst32Bit[i++] = abtPallet[clmbtDst[lPresentHeight + lX] * 4 + 1];
				clmbtDst32Bit[i++] = abtPallet[clmbtDst[lPresentHeight + lX] * 4 + 2];

				if( clmbtDst2[lPresentHeight + lX] < 0x10 )
				{
					clmbtDst32Bit[i++] = clmbtDst2[lPresentHeight + lX] * 16;
				}
				else
				{
					clmbtDst32Bit[i++] = 0xFF;
				}
			}
		}

		// 出力

		CImage				clImage;

		clImage.Init( pclArc, stGYUHeader.lWidth, stGYUHeader.lHeight, 32 );
		clImage.Write( &clmbtDst32Bit[0], dwDstSize32Bit );
	}
	else
	{
		// アルファ値が存在しない

		CImage				clImage;

		clImage.Init( pclArc, stGYUHeader.lWidth, stGYUHeader.lHeight, stGYUHeader.dwBpp, abtPallet, sizeof(abtPallet) );
		clImage.Write( &clmbtDst[0], dwDstSize );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	GYUの復号

BOOL	CRetouch::DecryptGYU(
	void*				pvSrc,							// 暗号データ
	DWORD				dwSrcSize,						// 暗号データサイズ
	DWORD				dwKey							// 復号キー
	)
{
	BYTE*				pbtSrc = (BYTE*)pvSrc;

	// テーブルの構築

	DWORD				adwTable[625 * 2];
	DWORD				dwWork;
	DWORD				dwWork2;
	BYTE				btTemp;

	dwWork = dwKey;

	for( DWORD i = 0 ; i < 624 ; i++ )
	{
		adwTable[i] = (dwWork & 0xFFFF0000);

		dwWork = (dwWork * 0x10DCD + 1);

		adwTable[i] |= (dwWork >> 16);

		dwWork = (dwWork * 0x10DCD + 1);
	}

	// 第一フェーズの準備

	ULARGE_INTEGER		astuliWork[8];

	astuliWork[4].LowPart = 0x9908B0DF;
	astuliWork[4].HighPart = 0x9908B0DF;

	astuliWork[5].LowPart = 1;
	astuliWork[5].HighPart = 1;

	astuliWork[6].QuadPart = (astuliWork[5].QuadPart << 31);

	astuliWork[7].QuadPart = astuliWork[6].QuadPart;
	astuliWork[7].LowPart -= astuliWork[5].LowPart;
	astuliWork[7].HighPart -= astuliWork[5].HighPart;

	astuliWork[0].QuadPart = (*(UINT64*)&adwTable[0] & astuliWork[6].QuadPart);
	astuliWork[1].QuadPart = (*(UINT64*)&adwTable[1] & astuliWork[7].QuadPart);
	astuliWork[2].QuadPart = *(UINT64*)&adwTable[397];

	// 第一フェーズ

	for( DWORD i = 0, j = 397 ; (i < 226) && (j < 623) ; i += 2, j += 2 )
	{
		astuliWork[0].QuadPart |= astuliWork[1].QuadPart;
		astuliWork[0].LowPart >>= 1;
		astuliWork[0].HighPart >>= 1;

		astuliWork[2].QuadPart ^= astuliWork[0].QuadPart;

		astuliWork[3].QuadPart = (astuliWork[1].QuadPart & astuliWork[5].QuadPart);
		astuliWork[3].LowPart -= astuliWork[5].LowPart;
		astuliWork[3].HighPart -= astuliWork[5].HighPart;
		astuliWork[3].QuadPart = ((~astuliWork[3].QuadPart & astuliWork[4].QuadPart) ^ astuliWork[2].QuadPart);

		astuliWork[0].QuadPart = (*(UINT64*)&adwTable[i + 2] & astuliWork[6].QuadPart);
		astuliWork[1].QuadPart = (*(UINT64*)&adwTable[i + 3] & astuliWork[7].QuadPart);
		astuliWork[2].QuadPart = *(UINT64*)&adwTable[j + 2];

		*(UINT64*)&adwTable[i] = astuliWork[3].QuadPart;
	}

	dwWork = (adwTable[226] & 0x80000000);
	dwWork2 = (adwTable[227] & 0x7FFFFFFF);

	dwWork = ((((dwWork | dwWork2) >> 1) ^ adwTable[623]) ^ 0x9908B0DF);
	dwWork2 = (((dwWork2 | 0xFFFFFFFE) + 1) & 0x9908B0DF);

	adwTable[226] = (dwWork ^ dwWork2);

	// 第二フェーズの準備

	adwTable[624] = adwTable[0];

	astuliWork[0].QuadPart = (*(UINT64*)&adwTable[227] & astuliWork[6].QuadPart);
	astuliWork[1].QuadPart = (*(UINT64*)&adwTable[228] & astuliWork[7].QuadPart);
	astuliWork[2].QuadPart = *(UINT64*)&adwTable[0];

	// 第二フェーズ

	for( DWORD i = 227, j = 0 ; (i < 625) && (j < 398) ; i += 2, j += 2 )
	{
		astuliWork[0].QuadPart |= astuliWork[1].QuadPart;
		astuliWork[0].LowPart >>= 1;
		astuliWork[0].HighPart >>= 1;

		astuliWork[2].QuadPart ^= astuliWork[0].QuadPart;

		astuliWork[3].QuadPart = (astuliWork[1].QuadPart & astuliWork[5].QuadPart);
		astuliWork[3].LowPart -= astuliWork[5].LowPart;
		astuliWork[3].HighPart -= astuliWork[5].HighPart;
		astuliWork[3].QuadPart = ((~astuliWork[3].QuadPart & astuliWork[4].QuadPart) ^ astuliWork[2].QuadPart);

		astuliWork[0].QuadPart = (*(UINT64*)&adwTable[i + 2] & astuliWork[6].QuadPart);
		astuliWork[1].QuadPart = (*(UINT64*)&adwTable[i + 3] & astuliWork[7].QuadPart);
		astuliWork[2].QuadPart = *(UINT64*)&adwTable[j + 2];

		*(UINT64*)&adwTable[i] = astuliWork[3].QuadPart;
	}

	// 第三フェーズの準備

	astuliWork[6].LowPart = 0x9D2C5680;
	astuliWork[6].HighPart = 0x9D2C5680;

	astuliWork[7].LowPart = 0xEFC60000;
	astuliWork[7].HighPart = 0xEFC60000;

	astuliWork[0].QuadPart = *(UINT64*)&adwTable[0];
	astuliWork[1].QuadPart = *(UINT64*)&adwTable[0];
	astuliWork[2].QuadPart = *(UINT64*)&adwTable[2];

	astuliWork[0].LowPart >>= 11;
	astuliWork[0].HighPart >>= 11;
	astuliWork[0].QuadPart ^= *(UINT64*)&adwTable[0];

	// 第三フェーズ

	for( DWORD i = 0, j = 625 ; (i < 624) && (j < 1249) ; i += 4, j += 4 )
	{
		// 

		astuliWork[1].QuadPart = astuliWork[0].QuadPart;
		astuliWork[3].QuadPart = astuliWork[2].QuadPart;

		astuliWork[0].LowPart <<= 7;
		astuliWork[0].HighPart <<= 7;
		astuliWork[0].QuadPart &= astuliWork[6].QuadPart;
		astuliWork[0].QuadPart ^= astuliWork[1].QuadPart;

		astuliWork[2].LowPart >>= 11;
		astuliWork[2].HighPart >>= 11;
		astuliWork[2].QuadPart ^= astuliWork[3].QuadPart;

		astuliWork[3].QuadPart = astuliWork[2].QuadPart;
		astuliWork[1].QuadPart = astuliWork[0].QuadPart;

		astuliWork[2].LowPart <<= 7;
		astuliWork[2].HighPart <<= 7;
		astuliWork[2].QuadPart &= astuliWork[6].QuadPart;
		astuliWork[2].QuadPart ^= astuliWork[3].QuadPart;

		astuliWork[0].LowPart <<= 15;
		astuliWork[0].HighPart <<= 15;
		astuliWork[0].QuadPart &= astuliWork[7].QuadPart;
		astuliWork[0].QuadPart ^= astuliWork[1].QuadPart;

		astuliWork[3].QuadPart = astuliWork[2].QuadPart;
		astuliWork[1].QuadPart = astuliWork[0].QuadPart;

		astuliWork[2].LowPart <<= 15;
		astuliWork[2].HighPart <<= 15;
		astuliWork[2].QuadPart &= astuliWork[7].QuadPart;

		astuliWork[0].LowPart >>= 18;
		astuliWork[0].HighPart >>= 18;

		astuliWork[1].QuadPart ^= astuliWork[0].QuadPart;
		astuliWork[2].QuadPart ^= astuliWork[3].QuadPart;

		astuliWork[0].QuadPart = *(INT64*)&adwTable[i + 4];
		astuliWork[3].QuadPart = astuliWork[2].QuadPart;

		*(INT64*)&adwTable[j] = astuliWork[1].QuadPart;

		// 

		astuliWork[2].LowPart >>= 18;
		astuliWork[2].HighPart >>= 18;

		astuliWork[3].QuadPart ^= astuliWork[2].QuadPart;

		astuliWork[1].QuadPart = astuliWork[0].QuadPart;
		astuliWork[2].QuadPart = *(INT64*)&adwTable[i + 6];

		astuliWork[0].LowPart >>= 11;
		astuliWork[0].HighPart >>= 11;
		astuliWork[0].QuadPart ^= astuliWork[1].QuadPart;

		*(INT64*)&adwTable[j + 2] = astuliWork[3].QuadPart;
	}

	// 復号

	for( DWORD i = 0, j = 625 ; i < 10 ; i++ )
	{
		dwWork = (adwTable[j++] % dwSrcSize);
		dwWork2 = (adwTable[j++] % dwSrcSize);

		btTemp = pbtSrc[dwWork];
		pbtSrc[dwWork] = pbtSrc[dwWork2];
		pbtSrc[dwWork2] = btTemp;
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	GYUの解凍

BOOL	CRetouch::DecompGYU(
	void*				pvDst,							// 格納先
	DWORD				dwDstSize,						// 格納先のサイズ
	const void*			pvSrc,							// 圧縮データ
	DWORD				dwSrcSize						// 圧縮データサイズ
	)
{
	// LZSSの解凍

	CLZSS				clLZSS;

	clLZSS.Decomp( pvDst, dwDstSize, pvSrc, dwSrcSize, 4096, 4078, 3 );

	return	TRUE;
}
