
#include	"stdafx.h"
#include	"../../Image.h"
#include	"JBP1.h"
#include	"PB2A.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード
//
//	備考	pvSrcは先頭部分が復号される

BOOL	CPB2A::Decode(
	CArcFile*			pclArc,							// アーカイブ
	void*				pvSrc,							// 入力データ
	DWORD				dwSrcSize						// 入力データサイズ
	)
{
	BYTE*				pbtSrc = (BYTE*) pvSrc;

	// 復号

	Decrypt( pbtSrc, dwSrcSize );

	// 画像情報の取得

	WORD				wType = *(WORD*) &pbtSrc[16];
	long				lWidth = *(WORD*) &pbtSrc[18];
	long				lHeight = *(WORD*) &pbtSrc[20];
	WORD				wBpp = *(WORD*) &pbtSrc[22];

	// 解凍

	switch( wType )
	{
		case	1:
			Decode1( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
			break;

		case	2:
			Decode2( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
			break;

		case	3:
		case	4:
			Decode4( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
			break;

		case	5:
			Decode5( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
			break;

		case	6:
			Decode6( pclArc, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );
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

void	CPB2A::Decrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwSize							// 復号サイズ
	)
{
	for( int i = 8 ; i < 32 ; i += 2 )
	{
		*(WORD*) &pbtTarget[i] ^= *(WORD*) &pbtTarget[dwSize - 3];
	}

	for( int i = 8, j = 0 ; i < 32 ; i++, j++ )
	{
		pbtTarget[i] -= pbtTarget[dwSize - 27 + j];
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード1

BOOL	CPB2A::Decode1(
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
//	デコード1

BOOL	CPB2A::Decode2(
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

	Decomp2( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );

	// 出力

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmbtDst[0], dwDstSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード4

BOOL	CPB2A::Decode4(
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

	Decomp4( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp );

	// 出力

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmbtDst[0], dwDstSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード5

BOOL	CPB2A::Decode5(
	CArcFile*			pclArc,							// アーカイブ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	// ベース画像用バッファ確保

	DWORD				dwBaseSize = (lWidth * lHeight * 4);

	YCMemory<BYTE>		clmbtBase( dwBaseSize );

	// ベース画像の解凍

	Decomp5( &clmbtBase[0], dwBaseSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp, NULL, 0 );

	// ベース画像の出力

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, 32 );
	clImage.WriteReverse( &clmbtBase[0], dwBaseSize );
	clImage.Close();

	// 画像数の取得

	DWORD				dwImageFiles = *(DWORD*) &pbtSrc[8];

	if( dwImageFiles == 1 )
	{
		// ベース画像しか存在しない

		return	TRUE;
	}

	// 差分画像用バッファ確保

	DWORD				dwDstSize = dwBaseSize;

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// 差分画像の出力

	for( DWORD dwFrameNumber = 1 ; dwFrameNumber < dwImageFiles ; dwFrameNumber++ )
	{
		// 差分画像の解凍

		Decomp5( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, wBpp, &clmbtBase[0], dwFrameNumber );

		// 差分画像の出力

		TCHAR				szDiffName[256];

		_stprintf( szDiffName, _T("_%02d"), (dwFrameNumber - 1) );

		clImage.Init( pclArc, lWidth, lHeight, 32, NULL, 0, szDiffName );
		clImage.WriteReverse( &clmbtDst[0], dwDstSize, FALSE );
		clImage.Close();
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード6

BOOL	CPB2A::Decode6(
	CArcFile*			pclArc,							// アーカイブ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	// ベース画像用バッファ確保

	DWORD				dwBaseSize = (lWidth * lHeight * 4);

	YCMemory<BYTE>		clmbtBase( dwBaseSize );

	// ベース画像の解凍

	Decomp6( &clmbtBase[0], dwBaseSize, pbtSrc, dwSrcSize, lWidth, lHeight, 32, NULL, 0 );

	// ベース画像の出力

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, 32 );
	clImage.WriteReverse( &clmbtBase[0], dwBaseSize );
	clImage.Close();

	// 画像数の取得

	DWORD				dwImageFiles = *(DWORD*) &pbtSrc[8];

	if( dwImageFiles == 1 )
	{
		// ベース画像しか存在しない

		return	TRUE;
	}

	// 差分画像用バッファ確保

	DWORD				dwDstSize = dwBaseSize;

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// 差分画像の出力

	for( DWORD dwFrameNumber = 1 ; dwFrameNumber < dwImageFiles ; dwFrameNumber++ )
	{
		// 差分画像の解凍

		Decomp6( &clmbtDst[0], dwDstSize, pbtSrc, dwSrcSize, lWidth, lHeight, 32, &clmbtBase[0], dwFrameNumber );

		// 差分画像の出力

		TCHAR				szDiffName[256];

		_stprintf( szDiffName, _T("_%02d"), (dwFrameNumber - 1) );

		clImage.Init( pclArc, lWidth, lHeight, 32, NULL, 0, szDiffName );
		clImage.WriteReverse( &clmbtDst[0], dwDstSize, FALSE );
		clImage.Close();
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	解凍1

BOOL	CPB2A::Decomp1(
	BYTE*				pbtDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	DWORD				dwOffsetOfFlags = *(DWORD*) &pbtSrc[24];
	DWORD				dwOffsetOfCompData = *(DWORD*) &pbtSrc[28];
	long				lWidthOfBlock = 8;
	long				lHeightOfBlock = 8;
	WORD				wByteCount = (wBpp >> 3);
	long				lLine = (lWidth * wByteCount);

	// LZSS解凍用バッファの確保

	DWORD				dwTempSize = (lWidth * lHeight * wByteCount);
	DWORD				dwTempPtr = 0;

	YCMemory<BYTE>		clmbtTemp( dwTempSize );

	// LZSS解凍

	const BYTE*			pbtFlags = &pbtSrc[dwOffsetOfFlags];
	const BYTE*			pbtCompData = &pbtSrc[dwOffsetOfCompData];
	DWORD				dwFlagsSize = (dwOffsetOfCompData - dwOffsetOfFlags);
	DWORD				dwCompDataSize = (dwSrcSize - dwOffsetOfCompData);

	DecompLZSS( &clmbtTemp[0], dwTempSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize );

	// 色成分の入れ替え

	long				lBlockCountOfWidth = ((lWidth + (lWidthOfBlock - 1)) / lWidthOfBlock);
	long				lBlockCountOfHeight = ((lHeight + (lHeightOfBlock - 1)) / lHeightOfBlock);

	for( WORD wColor = 0 ; wColor < wByteCount ; wColor++ )
	{
		BYTE*				pbtWorkOfDst = &pbtDst[wColor];

		for( long lY = 0, lBlockY = 0 ; lBlockY < lBlockCountOfHeight ; lY += lHeightOfBlock, lBlockY++ )
		{
			BYTE*				pbtWorkOfDst2 = pbtWorkOfDst;
			long				lHeightOfBlockMax = ((lY + lHeightOfBlock) > lHeight) ? (lHeight - lY) : lHeightOfBlock;

			// 1列分のブロックを処理

			for( long lX = 0, lBlockX = 0 ; lBlockX < lBlockCountOfWidth ; lX += lWidthOfBlock, lBlockX++ )
			{
				BYTE*				pbtWorkOfDst3 = pbtWorkOfDst2;
				long				lWidthOfBlockMax = ((lX + lWidthOfBlock) > lWidth) ? (lWidth - lX) : lWidthOfBlock;

				// 1ブロックの処理

				for( long i = 0 ; i < lHeightOfBlockMax ; i++ )
				{
					for( long j = 0 ; j < lWidthOfBlockMax ; j++ )
					{
						pbtWorkOfDst3[j * wByteCount] = clmbtTemp[dwTempPtr++];
					}

					pbtWorkOfDst3 += lLine;
				}

				// 隣のブロックを指す

				pbtWorkOfDst2 += (lWidthOfBlock * wByteCount);
			}

			// 下のブロックを指す

			pbtWorkOfDst += (lLine * lHeightOfBlock);
		}
	}

	return	TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////
//	解凍2

BOOL	CPB2A::Decomp2(
	BYTE*				pbtDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp							// ビット数
	)
{
	DWORD				dwOffsetOfFlags = *(DWORD*) &pbtSrc[24];
	DWORD				dwOffsetOfCompData = *(DWORD*) &pbtSrc[28];
	long				lWidthOfBlock = 8;
	long				lHeightOfBlock = 8;
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

		long				lBlockCountOfWidth = ((lWidth + (lWidthOfBlock - 1)) / lWidthOfBlock);
		long				lBlockCountOfHeight = ((lHeight + (lHeightOfBlock - 1)) / lHeightOfBlock);
		BYTE				btCode = 0x80;
		DWORD				dwTempPtr = 0;
		DWORD				dwFlagsPtr = 0;
		DWORD				dwCompDataPtr = 0;

		pbtFlags = pbtWorkOfSrc + 12;
		pbtCompData = pbtWorkOfSrc + *(DWORD*) &pbtWorkOfSrc[0] + 12;

		BYTE*				pbtWorkOfDst = &pbtDst[wColor];

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
//	解凍4

BOOL	CPB2A::Decomp4(
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

	const BYTE*			pbtAlpha = pbtSrc + *(DWORD*) &pbtSrc[24];
	DWORD				dwAlphaSize = *(DWORD*) &pbtSrc[28];

	// 解凍

	CJBP1				clJBP1;

	clJBP1.Decomp( pbtDst, &pbtSrc[32], wBpp, pbtAlpha, dwAlphaSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	解凍5
//
//	備考	pbtBaseにNULLが指定された場合は差分合成を行わない

BOOL	CPB2A::Decomp5(
	BYTE*				pbtDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp,							// ビット数
	const BYTE*			pbtBase,						// ベース画像
	DWORD				dwFrameNumber					// フレーム番号
	)
{
	// 解凍

	if( dwFrameNumber == 0 )
	{
		// ベース画像

		// LZSS解凍用バッファの確保

		DWORD				dwTempSize = (lWidth * lHeight);

		YCMemory<BYTE>		aclmbtTemp[4];

		// LZSS解凍

		for( WORD i = 0 ; i < 4 ; i++ )
		{
			// LZSS解凍用バッファの確保

			aclmbtTemp[i].resize( dwTempSize );

			// LZSS解凍

			const BYTE*			pbtFlags = pbtSrc + 64 + *(DWORD*) &pbtSrc[i * 8 + 32];
			const BYTE*			pbtCompData = pbtSrc + 64 + *(DWORD*) &pbtSrc[i * 8 + 36];
			DWORD				dwFlagsSize = 0xFFFFFFFF;	// 不明
			DWORD				dwCompDataSize = 0xFFFFFFFF;	// 不明

			DecompLZSS( &aclmbtTemp[i][0], dwTempSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize );
		}

		for( DWORD i = 0 ; i < dwTempSize ; i++ )
		{
			pbtDst[i * 4 + 0] = aclmbtTemp[0][i];
			pbtDst[i * 4 + 1] = aclmbtTemp[1][i];
			pbtDst[i * 4 + 2] = aclmbtTemp[2][i];
			pbtDst[i * 4 + 3] = aclmbtTemp[3][i];
		}

		// 復号

		for( DWORD i = 0 ; i < dwTempSize ; i++ )
		{
			BYTE				btData1 = pbtDst[i * 4 + 2] ^ pbtDst[i * 4 + 3];
			BYTE				btData2 = pbtDst[i * 4 + 1] ^ btData1;

			pbtDst[i * 4 + 2] = btData1;
			pbtDst[i * 4 + 1] = btData2;
			pbtDst[i * 4 + 0] ^= btData2;
		}
	}
	else
	{
		// 差分画像

		const BYTE*			pbtWorkOfSrc = pbtSrc + *(DWORD*) &pbtSrc[12];

		for( DWORD i = 1 ; i < dwFrameNumber ; i++ )
		{
			pbtWorkOfSrc += *(DWORD*) &pbtWorkOfSrc[0];
		}

		// LZSS解凍用バッファの確保

		DWORD				dwLZSSDecodeSize = *(DWORD*) &pbtWorkOfSrc[12];

		YCMemory<BYTE>		clmbtTemp( dwLZSSDecodeSize );

		// LZSS解凍

		const BYTE*			pbtFlags = pbtWorkOfSrc + 16;
		const BYTE*			pbtCompData = pbtWorkOfSrc + 16 + *(DWORD*) &pbtWorkOfSrc[4];
		DWORD				dwFlagsSize = 0xFFFFFFFF;	// 不明
		DWORD				dwCompDataSize = 0xFFFFFFFF;	// 不明

		DecompLZSS( &clmbtTemp[0], dwLZSSDecodeSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize );

		// ベース画像をコピー

		if( pbtBase != NULL )
		{
			memcpy( pbtDst, pbtBase, dwDstSize );
		}
		else
		{
			memset( pbtDst, 0xFF, dwDstSize );
		}

		// 解凍

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
			// 1列分のブロックを処理

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

					for( int i = 0 ; i < lBlockOfHeightMax ; i++ )
					{
						for( int j = 0 ; j < lBlockOfWidthMax ; j++ )
						{
							memcpy( &pbtWorkOfDst3[j * 4], &pbtCompData[dwCompDataPtr + j * 3], 3 );

							pbtWorkOfDst3[j * 4 + 3] = 0xFF;
						}

						dwCompDataPtr += 3;
						pbtWorkOfDst3 += lLine;
					}
				}

				// 隣のブロックを指す

				pbtWorkOfDst2 += (lWidthOfBlock * 4);
				btCode >>= 1;
			}

			pbtWorkOfDst += (lLine * lHeightOfBlock);
		}
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	解凍6
//
//	備考	pbtBaseにNULLが指定された場合は差分合成を行わない

BOOL	CPB2A::Decomp6(
	BYTE*				pbtDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const BYTE*			pbtSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	long				lWidth,							// 横幅
	long				lHeight,						// 縦幅
	WORD				wBpp,							// ビット数
	const BYTE*			pbtBase,						// ベース画像
	DWORD				dwFrameNumber					// フレーム番号
	)
{
	// 解凍

	if( dwFrameNumber == 0 )
	{
		// ベース画像

		const BYTE*			pbtBaseOfSrc = pbtSrc + 36 + ((strlen( (char*) pbtSrc + 36 ) + 4) & 0xFFFFFFFC);

		// LZSS解凍用バッファの確保

		DWORD				dwTempSize = (lWidth * lHeight);

		YCMemory<BYTE>		aclmbtTemp[4];

		// LZSS解凍

		for( WORD i = 0 ; i < 4 ; i++ )
		{
			// LZSS解凍用バッファの確保

			aclmbtTemp[i].resize( dwTempSize );

			// LZSS解凍

			const BYTE*			pbtFlags = pbtBaseOfSrc + 32 + *(DWORD*) &pbtBaseOfSrc[i * 8];
			const BYTE*			pbtCompData = pbtBaseOfSrc + 32 + *(DWORD*) &pbtBaseOfSrc[i * 8 + 4];
			DWORD				dwFlagsSize = 0xFFFFFFFF;	// 不明
			DWORD				dwCompDataSize = 0xFFFFFFFF;	// 不明

			DecompLZSS( &aclmbtTemp[i][0], dwTempSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize );
		}

		for( DWORD i = 0 ; i < dwTempSize ; i++ )
		{
			pbtDst[i * 4 + 0] = aclmbtTemp[0][i];
			pbtDst[i * 4 + 1] = aclmbtTemp[1][i];
			pbtDst[i * 4 + 2] = aclmbtTemp[2][i];
			pbtDst[i * 4 + 3] = aclmbtTemp[3][i];
		}

		// 復号

		for( DWORD i = 0 ; i < dwTempSize ; i++ )
		{
			BYTE				btData1 = pbtDst[i * 4 + 2] ^ pbtDst[i * 4 + 3];
			BYTE				btData2 = pbtDst[i * 4 + 1] ^ btData1;

			pbtDst[i * 4 + 2] = btData1;
			pbtDst[i * 4 + 1] = btData2;
			pbtDst[i * 4 + 0] ^= btData2;
		}
	}
	else
	{
		// 差分画像

		const BYTE*			pbtSrcOfDiff = pbtSrc + *(DWORD*) &pbtSrc[32] + 32;

		for( DWORD i = 1 ; i < dwFrameNumber ; i++ )
		{
			pbtSrcOfDiff += *(DWORD*) &pbtSrcOfDiff[0];
		}

		// 差分画像ファイル名の取得

		char				szDiffName[256];

		size_t				uNameLen = ((strlen( (char*) pbtSrcOfDiff + 4) + 4 ) & 0xFFFFFFFC) + 4;

		strcpy( szDiffName, (char*) pbtSrcOfDiff + 4 );

		// LZSS解凍用バッファの確保

		DWORD				dwLZSSDecodeSize = *(DWORD*) &pbtSrcOfDiff[uNameLen + 8];

		YCMemory<BYTE>		clmbtTemp( dwLZSSDecodeSize );

		// LZSS解凍

		const BYTE*			pbtFlags = pbtSrcOfDiff + 16 + uNameLen;
		const BYTE*			pbtCompData = pbtSrcOfDiff + 16 + uNameLen + *(DWORD*) &pbtSrcOfDiff[uNameLen];
		DWORD				dwFlagsSize = 0xFFFFFFFF;	// 不明
		DWORD				dwCompDataSize = 0xFFFFFFFF;	// 不明

		DecompLZSS( &clmbtTemp[0], dwLZSSDecodeSize, pbtFlags, dwFlagsSize, pbtCompData, dwCompDataSize );

		// ベース画像をコピー

		if( pbtBase != NULL )
		{
			memcpy( pbtDst, pbtBase, dwDstSize );
		}
		else
		{
			memset( pbtDst, 0xFF, dwDstSize );
		}

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

			// 1列分のブロックを処理

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
	}

	return	TRUE;
}
