
#include	"stdafx.h"
#include	"../../ExtractBase.h"
#include	"PB.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	LZSSの解凍

BOOL	CPB::DecompLZSS(
	void*				pvDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const void*			pvFlags,						// フラグデータ
	DWORD				dwFlagsSize,					// フラグデータサイズ
	const void*			pvSrc,							// 圧縮データ
	DWORD				dwSrcSize						// 圧縮データサイズ
	)
{
	// 辞書の初期化

	DWORD				dwDicSize = 2048;

	YCMemory<BYTE>		clmbtDic( dwDicSize );

	ZeroMemory( &clmbtDic[0], dwDicSize );

	DWORD				dwDicPtr = 2014;

	// 解凍

	const BYTE*			pbtSrc = (const BYTE*) pvSrc;
	const BYTE*			pbtFlags = (const BYTE*) pvFlags;
	BYTE*				pbtDst = (BYTE*) pvDst;
	DWORD				dwSrcPtr = 0;
	DWORD				dwFlagsPtr = 0;
	DWORD				dwDstPtr = 0;
	BYTE				btCode = 0x80;

	while( (dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) && (dwFlagsPtr < dwFlagsSize) )
	{
		if( btCode == 0 )
		{
			dwFlagsPtr++;
			btCode = 0x80;
		}

		if( pbtFlags[dwFlagsPtr] & btCode )
		{
			// 圧縮されている

			WORD				wWork = *(WORD*) &pbtSrc[dwSrcPtr];

			DWORD				dwBack = (wWork >> 5);
			DWORD				dwLength = (wWork & 0x1F) + 3;

			// バッファを超えないように調整

			if( (dwDstPtr + dwLength) > dwDstSize )
			{
				dwLength = (dwDstSize - dwDstPtr);
			}

			// 辞書参照

			for( DWORD i = 0 ; i < dwLength ; i++ )
			{
				pbtDst[dwDstPtr + i] = clmbtDic[dwDicPtr++] = clmbtDic[dwBack++];

				dwDicPtr &= (dwDicSize - 1);
				dwBack &= (dwDicSize - 1);
			}

			dwSrcPtr += 2;
			dwDstPtr += dwLength;
		}
		else
		{
			// 無圧縮

			pbtDst[dwDstPtr++] = clmbtDic[dwDicPtr++] = pbtSrc[dwSrcPtr++];

			dwDicPtr &= (dwDicSize - 1);
		}

		btCode >>= 1;
	}

	return	TRUE;
}
