
#include	"stdafx.h"
#include	"../../Image.h"
#include	"JBP1.h"
#include	"PB3B.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード
//
//	備考	pvSrcは先頭部分が復号される

BOOL	CPB3B::Decode(
	CArcFile*			pclArc,							// アーカイブ
	void*				pvSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	PB3B_DECRYPT		pfnDecrypt						// 復号関数
	)
{
	BYTE*				pbtSrc = (BYTE*) pvSrc;

	// 復号

	Decrypt( pbtSrc, dwSrcSize );

	// 画像情報の取得

	WORD				wType = *(WORD*) &pbtSrc[28];
	long				lWidth = *(WORD*) &pbtSrc[30];
	long				lHeight = *(WORD*) &pbtSrc[32];
	WORD				wBpp = *(WORD*) &pbtSrc[34];

	switch( wType )
	{
		case	1:
			Decode1( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
			break;

		case	2:
		case	3:
			Decode3( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
			break;

		case	4:
			Decode4( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
			break;

		case	5:
			Decode5( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
			break;

		case	6:
			Decode6( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp, pfnDecrypt );
			break;

		default:
			// 不明

			pclArc->OpenFile();
			pclArc->WriteFile( pbtSrc, dwSrcSize );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号

void	CPB3B::Decrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwSize							// 復号サイズ
	)
{
	for( int i = 8 ; i < 52 ; i += 2 )
	{
		*(WORD*) &pbtTarget[i] ^= *(WORD*) &pbtTarget[dwSize - 3];
	}

	for( int i = 8, j = 0 ; i < 52 ; i++, j++ )
	{
		pbtTarget[i] -= pbtTarget[dwSize - 47 + j];
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード1

BOOL	CPB3B::Decode1(
	CArcFile*			pclArc,							// アーカイブ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	// 出力用バッファ確保

	DWORD				dwDstSize = (lWidth * lHeight * (wBpp >> 3));

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// 解凍

	Decomp1( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );

	// 出力

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmbtDst[0], dwDstSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード3

BOOL	CPB3B::Decode3(
	CArcFile*			pclArc,							// アーカイブ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	// 出力用バッファ確保

	DWORD				dwDstSize = (lWidth * lHeight * (wBpp >> 3));

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// 解凍

	Decomp3( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );

	// 出力

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmbtDst[0], dwDstSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード4

BOOL	CPB3B::Decode4(
	CArcFile*			pclArc,							// アーカイブ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	pclArc->OpenFile();
	pclArc->WriteFile( pbtSrc, dwSrcSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード5

BOOL	CPB3B::Decode5(
	CArcFile*			pclArc,							// アーカイブ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	// ベース画像用バッファ確保

	DWORD				dwBaseSize = (lWidth * lHeight * (wBpp >> 3));

	YCMemory<BYTE>		clmbtBase( dwBaseSize );

	// ベース画像の解凍

	Decomp5( &clmbtBase[0], dwBaseSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );

	// ベース画像の出力

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmbtBase[0], dwBaseSize );
	clImage.Close();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード6

BOOL	CPB3B::Decode6(
	CArcFile*			pclArc,							// アーカイブ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp,							// ビット数
	PB3B_DECRYPT		pfnDecrypt						// 復号関数
	)
{
	// 出力用バッファ確保

	DWORD				dwDstSize = (lWidth * lHeight * (wBpp >> 3));

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// ベース画像のファイル名取得

	BYTE				abtBaseFileName[64];

	memcpy( abtBaseFileName, pbtSrc + 52, 32 );

	// ベース画像のファイル名復号

	const static DWORD	adwCrypt[] =
	{
		0x9CF375A6, 0xA37869C5, 0x794FA53E, 0xC73AFE59
	};

	for( int i = 0 ; i < 2 ; i++ )
	{
		for( int j = 0 ; j < 4 ; j++ )
		{
			((DWORD*) abtBaseFileName)[i * 4 + j] ^= adwCrypt[j];
		}
	}

	abtBaseFileName[32] = '\0';

	YCString			clsBaseFileName = (char*) abtBaseFileName;

	clsBaseFileName += _T(".pb3");

	// プリミティブリンクのベース画像のファイル名が一部変になっているので修正

	if( clsBaseFileName == _T("ast-si0a_100.pb3") )
	{
		clsBaseFileName = _T("st-si0a_100a.pb3");
	}

	// ベース画像の格納

	const SFileInfo*		pstBaseFileInfo = pclArc->GetFileInfo( clsBaseFileName, TRUE );

	if( pstBaseFileInfo != NULL )
	{
		// ベース画像が存在する

		DWORD				dwBaseSrcSize = pstBaseFileInfo->sizeCmp;

		YCMemory<BYTE>		clmbtBaseSrc( dwBaseSrcSize );

		// ベース画像の読み込み

		pclArc->SeekHed( pstBaseFileInfo->start );
		pclArc->Read( &clmbtBaseSrc[0], dwBaseSrcSize );

		// ベース画像の復号

		pfnDecrypt( &clmbtBaseSrc[0], dwBaseSrcSize, pclArc, *pstBaseFileInfo );
		Decrypt( &clmbtBaseSrc[0], dwBaseSrcSize );

		// ベース画像の解凍

		Decomp5( &clmbtDst[0], dwDstSize, &clmbtBaseSrc[0], dwBaseSrcSize, lWidth, lHeight, wBpp );
	}
	else
	{
		// 差分画像のみ

		ZeroMemory( &clmbtDst[0], dwDstSize );
	}

	// 差分画像の解凍

	Decomp6( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );

	// 出力

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmbtDst[0], dwDstSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	解凍1

BOOL	CPB3B::Decomp1(
	BYTE*				pbtDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	DWORD				dwOffsetOfFlags = *(DWORD*) &pbtSrc[44];
	DWORD				dwOffsetOfCompData = *(DWORD*) &pbtSrc[48];
	long				lWidthOfBlock = *(DWORD*) &pbtSrc[24];
	long				lHeightOfBlock = *(DWORD*) &pbtSrc[24];
	WORD				wByteCount = (wBpp >> 3);
	long				lLine = (lWidth * wByteCount);

	// LZSS解凍用バッファの確保

	DWORD				dwTempSize = (lWidth * lHeight);

	YCMemory<BYTE>		clmbtTemp( dwTempSize );

	// 解凍

	for( WORD wColor = 0 ; wColor < wByteCount ; wColor++ )
	{
		DWORD				dwWork;

		// 

		const BYTE*			pbtWorkOfSrc = &pbtSrc[dwOffsetOfFlags];

		dwWork = (wByteCount * 4);

		for( WORD i = 0 ; i < wColor ; i++ )
		{
			dwWork += ((DWORD*) pbtWorkOfSrc)[i];
		}

		pbtWorkOfSrc += dwWork;

		// LZSS解凍後のサイズを取得

		DWORD				dwLZSSDecodeSize = *(DWORD*) &pbtWorkOfSrc[8];

		// フラグデータへのポインタの取得

		const BYTE*			pbtFlags = pbtWorkOfSrc + *(DWORD*) &pbtWorkOfSrc[0] + *(DWORD*) &pbtWorkOfSrc[4] + 12;

		// 圧縮データへのポインタの取得

		const BYTE*			pbtCompData = &pbtSrc[dwOffsetOfCompData];

		dwWork = (wByteCount * 4);

		for( WORD i = 0 ; i < wColor ; i++ )
		{
			dwWork += ((DWORD*) pbtCompData)[i];
		}

		pbtCompData += dwWork;

		// バッファ不足に対処
		// dwLZSSDecodeSizeで毎回バッファ確保すれば対処する必要ないけど、メモリ確保／破棄は最小限に抑えたい

		if( dwTempSize < dwLZSSDecodeSize )
		{
			clmbtTemp.resize( dwLZSSDecodeSize );
		}

		// LZSS解凍

		DWORD				dwFlagsSize = 0xFFFFFFFF;	// サイズ不明
		DWORD				dwCompDataSize = 0xFFFFFFFF;	// サイズ不明

		DecompLZSS( &clmbtTemp[0], dwLZSSDecodeSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize );

		// ブロック圧縮の解凍

		BYTE*				pbtWorkOfDst = &pbtDst[wColor];
		long				lBlockCountOfWidth = ((lWidth + (lWidthOfBlock - 1)) / lWidthOfBlock);
		long				lBlockCountOfHeight = ((lHeight + (lHeightOfBlock - 1)) / lHeightOfBlock);
		BYTE				btCode = 0x80;
		DWORD				dwTempPtr = 0;
		DWORD				dwFlagsPtr = 0;
		DWORD				dwCompDataPtr = 0;

		pbtFlags = pbtWorkOfSrc + 12;
		pbtCompData = pbtWorkOfSrc + *(DWORD*) &pbtWorkOfSrc[0] + 12;

		for( long lY = 0, lBlockY = 0 ; lBlockY < lBlockCountOfHeight ; lY += lHeightOfBlock, lBlockY++ )
		{
			BYTE*				pbtWorkOfDst2 = pbtWorkOfDst;
			long				lHeightOfBlockMax = ((lY + lHeightOfBlock) > lHeight) ? (lHeight - lY) : lHeightOfBlock;

			for( long lX = 0, lBlockX = 0 ; lBlockX < lBlockCountOfWidth ; lX += lWidthOfBlock, lBlockX++ )
			{
				if( btCode == 0 )
				{
					// 8ブロック処理した

					dwFlagsPtr++;
					btCode = 0x80;
				}

				// 1ブロックの処理

				BYTE*				pbtWorkOfDst3 = pbtWorkOfDst2;
				long				lWidthOfBlockMax = ((lX + lWidthOfBlock) > lWidth) ? (lWidth - lX) : lWidthOfBlock;

				if( pbtFlags[dwFlagsPtr] & btCode )
				{
					// 圧縮されている

					for( long i = 0 ; i < lHeightOfBlockMax ; i++ )
					{
						for( long j = 0 ; j < lWidthOfBlockMax ; j++ )
						{
							pbtWorkOfDst3[j * wByteCount] = pbtCompData[dwCompDataPtr];
						}

						pbtWorkOfDst3 += lLine;
					}

					dwCompDataPtr++;
				}
				else
				{
					// 圧縮されていない

					for( long i = 0 ; i < lHeightOfBlockMax ; i++ )
					{
						for( long j = 0 ; j < lWidthOfBlockMax ; j++ )
						{
							pbtWorkOfDst3[j * wByteCount] = clmbtTemp[dwTempPtr++];
						}

						pbtWorkOfDst3 += lLine;
					}
				}

				// 隣のブロックを指す

				pbtWorkOfDst2 += (lWidthOfBlock * wByteCount);
				btCode >>= 1;
			}

			pbtWorkOfDst += (lLine * lHeightOfBlock);
		}
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	解凍3

BOOL	CPB3B::Decomp3(
	BYTE*				pbtDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	// アルファ値の取得

	const BYTE*			pbtAlpha = pbtSrc + *(DWORD*) &pbtSrc[44];
	DWORD				dwAlphaSize = *(DWORD*) &pbtSrc[48];

	// 解凍

	CJBP1				clJBP1;

	clJBP1.Decomp( pbtDst, &pbtSrc[52], wBpp, pbtAlpha, dwAlphaSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	解凍4

BOOL	CPB3B::Decomp4(
	BYTE*				pbtDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	解凍5

BOOL	CPB3B::Decomp5(
	BYTE*				pbtDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	// LZSS解凍用バッファ

	DWORD				dwTempSize = (lWidth * lHeight);

	YCMemory<BYTE>		aclmbtTemp[4];

	// ベース画像の解凍

	for( int i = 0 ; i < 4 ; i++ )
	{
		// LZSS解凍用バッファの確保

		aclmbtTemp[i].resize( dwTempSize );

		// フラグデータへのポインタの取得

		const BYTE*			pbtFlags = pbtSrc + 84 + *(DWORD*) &pbtSrc[i * 8 + 52];

		// 圧縮データへのポインタの取得

		const BYTE*			pbtCompData = pbtSrc + 84 + *(DWORD*) &pbtSrc[i * 8 + 56];

		// LZSS解凍

		DWORD				dwFlagsSize = 0xFFFFFFFF;	// サイズ不明
		DWORD				dwCompDataSize = 0xFFFFFFFF;	// サイズ不明

		DecompLZSS( &aclmbtTemp[i][0], dwTempSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize );
	}

	// 格納

	pbtDst[0] = aclmbtTemp[0][0];
	pbtDst[1] = aclmbtTemp[1][0];
	pbtDst[2] = aclmbtTemp[2][0];
	pbtDst[3] = aclmbtTemp[3][0];

	for( DWORD i = 1 ; i < dwTempSize ; i++ )
	{
		pbtDst[i * 4 + 0] = aclmbtTemp[0][i] + pbtDst[i * 4 - 4];
		pbtDst[i * 4 + 1] = aclmbtTemp[1][i] + pbtDst[i * 4 - 3];
		pbtDst[i * 4 + 2] = aclmbtTemp[2][i] + pbtDst[i * 4 - 2];
		pbtDst[i * 4 + 3] = aclmbtTemp[3][i] + pbtDst[i * 4 - 1];
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	解凍6
//
//	備考	pbtDstにベース画像が格納されていることを前提にしている

BOOL	CPB3B::Decomp6(
	BYTE*				pbtDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	// LZSS解凍用バッファの確保

	DWORD				dwLZSSDecodeSize = *(DWORD*) &pbtSrc[24];

	YCMemory<BYTE>		clmbtTemp( dwLZSSDecodeSize );

	// LZSS解凍

	const BYTE*			pbtFlags = pbtSrc + 84;
	const BYTE*			pbtCompData = pbtSrc + 84 + *(DWORD*) &pbtSrc[44];
	DWORD				dwFlagsSize = 0xFFFFFFFF;	// 不明
	DWORD				dwCompDataSize = 0xFFFFFFFF;	// 不明

	DecompLZSS( &clmbtTemp[0], dwLZSSDecodeSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize );

	// 解凍

	WORD				wByteCount = (wBpp >> 3);
	long				lLine = (lWidth * 4);
	long				lWidthOfBlock = 8;
	long				lHeightOfBlock = 8;
	long				lBlockCountOfWidth = ((lWidth + (lWidthOfBlock - 1)) / lWidthOfBlock);
	long				lBlockCountOfHeight = ((lHeight + (lHeightOfBlock - 1)) / lHeightOfBlock);
	DWORD				dwFlagsPtr = 0;
	DWORD				dwCompDataPtr = 0;
	BYTE				btCode = 0x80;

	pbtFlags = &clmbtTemp[8];
	pbtCompData = &clmbtTemp[*(DWORD*) &clmbtTemp[0] + 8];

	BYTE*				pbtWorkOfDst = pbtDst;

	for( long lY = 0, lBlockY = 0 ; lBlockY < lBlockCountOfHeight ; lY += lHeightOfBlock, lBlockY++ )
	{
		BYTE*				pbtWorkOfDst2 = pbtWorkOfDst;
		long				lBlockOfHeightMax = ((lY + lHeightOfBlock) > lHeight) ? (lHeight - lY) : lHeightOfBlock;

		for( long lX = 0, lBlockX = 0 ; lBlockX < lBlockCountOfWidth ; lX += lWidthOfBlock, lBlockX++ )
		{
			if( btCode == 0 )
			{
				// 8ブロック処理した

				btCode = 0x80;
				dwFlagsPtr++;
			}

			// 1ブロックの処理

			BYTE*				pbtWorkOfDst3 = pbtWorkOfDst2;
			long				lBlockOfWidthMax = ((lX + lWidthOfBlock) > lWidth) ? (lWidth - lX) : lWidthOfBlock;

			if( (pbtFlags[dwFlagsPtr] & btCode) == 0 )
			{
				// 差分

				for( long i = 0 ; i < lBlockOfHeightMax ; i++ )
				{
					for( long j = 0 ; j < lBlockOfWidthMax ; j++ )
					{
						memcpy( &pbtWorkOfDst3[j * 4], &pbtCompData[dwCompDataPtr + j * 4], 4 );
					}

					dwCompDataPtr += (lBlockOfWidthMax * 4);
					pbtWorkOfDst3 += lLine;
				}
			}

			// 隣のブロックを指す

			pbtWorkOfDst2 += (lWidthOfBlock * 4);
			btCode >>= 1;
		}

		pbtWorkOfDst += (lLine * lHeightOfBlock);
	}

	return	TRUE;
}
