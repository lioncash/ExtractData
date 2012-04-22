
#include	"stdafx.h"
#include	"../ExtractBase.h"
#include	"../bzip2/bzlib.h"
#include	"../Image.h"
#include	"Standard.h"
#include	"Nscr.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CNscr::Mount(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( MountNsa( pclArc ) )
	{
		return	TRUE;
	}

	if( MountSar( pclArc) )
	{
		return	TRUE;
	}

	if( MountScr( pclArc) )
	{
		return	TRUE;
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	nsaのマウント

BOOL	CNscr::MountNsa(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pclArc->GetArcExten() != _T(".nsa") )
	{
		return	FALSE;
	}

	// ファイル数取得

	DWORD				dwFiles = 0;

	if( memcmp( pclArc->GetHed(), "\0\0", 2 ) == 0 )
	{
		// 4バイトと断定

		pclArc->Read( &dwFiles, 4 );
	}
	else
	{
		pclArc->Read( &dwFiles, 2 );
		dwFiles <<= 16;
	}

	pclArc->ConvEndian( &dwFiles );

	// オフセット取得

	DWORD				dwOffset;

	pclArc->Read( &dwOffset, 4 );
	pclArc->ConvEndian( &dwOffset );

	if( memcmp( pclArc->GetHed(), "\0\0", 2 ) == 0 )
	{
		dwOffset += 2;
	}

	// インデックスサイズ取得

	DWORD				dwIndexSize = dwOffset;

	dwIndexSize -= (memcmp( pclArc->GetHed(), "\0\0", 2 ) == 0) ? 8 : 6;

	// インデックス取得

	YCMemory<BYTE>		clmbtIndex( dwIndexSize );
	DWORD				dwIndexPtr = 0;

	pclArc->Read( &clmbtIndex[0], dwIndexSize );

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// ファイル名取得

		TCHAR				szFileName[256];

		lstrcpy( szFileName, (LPCTSTR) &clmbtIndex[dwIndexPtr] );
		dwIndexPtr += lstrlen( szFileName ) + 1;

		BYTE				btType = clmbtIndex[dwIndexPtr];

		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.start = pclArc->ConvEndian( *(DWORD*) &clmbtIndex[dwIndexPtr + 1] ) + dwOffset;
		stFileInfo.sizeCmp = pclArc->ConvEndian( *(DWORD*) &clmbtIndex[dwIndexPtr + 5] );
		stFileInfo.sizeOrg = pclArc->ConvEndian( *(DWORD*) &clmbtIndex[dwIndexPtr + 9] );
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		switch( btType )
		{
		case	1:
			stFileInfo.format = _T("SPB");
			break;

		case	2:
			stFileInfo.format = _T("LZSS");
			break;
		}

		pclArc->AddFileInfo( stFileInfo );

		dwIndexPtr += 13;
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	sarのマウント

BOOL	CNscr::MountSar(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcExten() != _T(".sar") )
	{
		return	FALSE;
	}

	// ファイル数取得

	WORD				dwFiles;

	pclArc->Read( &dwFiles, 2 );
	pclArc->ConvEndian( &dwFiles );

	// オフセット取得

	DWORD				dwOffset;

	pclArc->Read( &dwOffset, 4 );
	pclArc->ConvEndian( &dwOffset );

	// インデックスサイズ取得

	DWORD				dwIndexSize = dwOffset - 6;

	// インデックス取得

	YCMemory<BYTE>		clmbtIndex( dwIndexSize );
	DWORD				dwIndexPtr = 0;

	pclArc->Read( &clmbtIndex[0], dwIndexSize );

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// ファイル名取得

		TCHAR				szFileName[256];

		lstrcpy( szFileName, (LPCTSTR) &clmbtIndex[dwIndexPtr] );
		dwIndexPtr += lstrlen( szFileName ) + 1;

		// リストビューに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.start = pclArc->ConvEndian( *(DWORD*) &clmbtIndex[dwIndexPtr + 0] ) + dwOffset;
		stFileInfo.sizeOrg = pclArc->ConvEndian( *(DWORD*) &clmbtIndex[dwIndexPtr + 4] );
		stFileInfo.sizeCmp = stFileInfo.sizeOrg;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );

		dwIndexPtr += 8;
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	スクリプトファイルのマウント

BOOL	CNscr::MountScr(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->GetArcName() != _T("nscript.dat") )
	{
		return	FALSE;
	}

	return	pclArc->Mount();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード

BOOL	CNscr::Decode(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( DecodeScr( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodeSPB( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodeLZSS( pclArc) )
	{
		return	TRUE;
	}

	if( DecodeNBZ( pclArc) )
	{
		return	TRUE;
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	スクリプトファイルのデコード

BOOL	CNscr::DecodeScr(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->name != _T("nscript.dat") )
	{
		return	FALSE;
	}

	// バッファ確保

	DWORD				dwBufferSize = pclArc->GetBufSize();

	YCMemory<BYTE>		clmbtBuffer( dwBufferSize );
	YCMemory<BYTE>		clmbtBuffer2( dwBufferSize * 2 );

	// 出力ファイル生成

	pclArc->OpenFile( _T(".txt") );

	for( DWORD dwWriteSize = 0 ; dwWriteSize != pstFileInfo->sizeOrg ; dwWriteSize += dwBufferSize )
	{
		// バッファ調整

		pclArc->SetBufSize( &dwBufferSize, dwWriteSize );

		// 読み書き

		pclArc->Read( &clmbtBuffer[0], dwBufferSize );

		DWORD				dwBufferSize2 = 0;

		for( DWORD i = 0 ; i < dwBufferSize ; i++ )
		{
			// 復号

			clmbtBuffer2[dwBufferSize2] = clmbtBuffer[i] ^ 0x84;

			// 改行コードをCR+LFに変更

			if( clmbtBuffer2[dwBufferSize2] == '\n' )
			{
				clmbtBuffer2[dwBufferSize2++] = '\r';
				clmbtBuffer2[dwBufferSize2] = '\n';
			}

			dwBufferSize2++;
		}

		pclArc->WriteFile( &clmbtBuffer2[0], dwBufferSize2 );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	NBZの解凍

BOOL	CNscr::DecodeNBZ(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->format != _T("NBZ") )
	{
		return	FALSE;
	}

	// ファイルサイズ取得

	DWORD				dwDstSize;

	pclArc->Read( &dwDstSize, 4 );
	pclArc->ConvEndian( &dwDstSize );

	DWORD				dwSrcSize = (pstFileInfo->sizeCmp - 4);

	// バッファ確保

	YCMemory<BYTE>		clmbtSrc( dwSrcSize );
	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// NBZ展開

	pclArc->Read( &clmbtSrc[0], dwSrcSize );

	BZ2_bzBuffToBuffDecompress( (char*) &clmbtDst[0], &(UINT&) dwDstSize, (char*) &clmbtSrc[0], dwSrcSize, 0, 0 );

	// ファイル内容から拡張子を取得

	YCString			clsFileExt;

	GetFileExt( clsFileExt, &clmbtDst[0] );

	// 出力

	pclArc->OpenFile( clsFileExt );
	pclArc->WriteFile( &clmbtDst[0], dwDstSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	拡張子の取得

void	CNscr::GetFileExt(
	YCString&			rfclsDst,						// 格納先
	const BYTE*			pbtBuffer						// バッファ
	)
{
	if( memcmp( pbtBuffer, "BM", 2 ) == 0 )
	{
		rfclsDst = _T(".bmp");
	}
	else if( (memcmp( pbtBuffer, "RIFF", 4) == 0) && (memcmp( &pbtBuffer[8], "WAVEfmt ", 8 ) == 0) )
	{
		rfclsDst = _T(".wav");
	}
	else if( memcmp( pbtBuffer, "MThd", 4 ) == 0 )
	{
		rfclsDst = _T(".mid");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ビット列の取得

DWORD	CNscr::GetBit(
	const BYTE*			pbtSrc,							// 入力データ
	DWORD				dwReadBitLength,				// 読み込むビット数
	DWORD*				pdwReadByteLength				// 読み込んだバイト数
	)
{
	DWORD				dwResult = 0;
	DWORD				dwSrcPtr = 0;

	for( DWORD i = 0 ; i < dwReadBitLength ; i++ )
	{
		if( btMaskForGetBit == 0 )
		{
			btSrcForGetBit = pbtSrc[dwSrcPtr++];
			btMaskForGetBit = 0x80;
		}

		dwResult <<= 1;

		if( btSrcForGetBit & btMaskForGetBit )
		{
			dwResult++;
		}

		btMaskForGetBit >>= 1;
	}

	*pdwReadByteLength = dwSrcPtr;

	return	dwResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	SPBの解凍

BOOL	CNscr::DecodeSPB(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->format != _T("SPB") )
	{
		return	FALSE;
	}

	// 横幅の取得

	WORD				wWidth;

	pclArc->Read( &wWidth, 2 );
	pclArc->ConvEndian( &wWidth );

	// 縦幅の取得

	WORD				wHeight;

	pclArc->Read( &wHeight, 2 );
	pclArc->ConvEndian( &wHeight );

	// 画像のパラメータの取得

	DWORD				dwColorSize = (wWidth * wHeight);
	DWORD				dwLine = (wWidth * 3);
	DWORD				dwPitch = ((dwLine + 3) & 0xFFFFFFFC);

	// バッファ確保

	DWORD				dwSrcSize = pstFileInfo->sizeCmp - 4;
	DWORD				dwDstSize = pstFileInfo->sizeOrg - 54;
	DWORD				dwWorkSize = (dwColorSize + 4) * 3;

	YCMemory<BYTE>		clmbtSrc( dwSrcSize );
	YCMemory<BYTE>		clmbtDst( dwDstSize );
	YCMemory<BYTE>		clmbtWork( dwWorkSize );

	// SPB読み込み

	pclArc->Read( &clmbtSrc[0], dwSrcSize );

	// SPB展開

	DWORD				dwSrcPtr = 0;
	DWORD				dwDstPtr = 0;
	DWORD				dwWorkPtr = 0;
	DWORD				adwWorkPtrForSave[3];

	btMaskForGetBit = 0;
	btSrcForGetBit = 0;

	for( int i = 0 ; i < 3 ; i++ )
	{
		DWORD				dwReadByteLength;
		DWORD				dwData;
		DWORD				dwWork;

		adwWorkPtrForSave[i] = dwWorkPtr;

		dwData = GetBit( &clmbtSrc[dwSrcPtr], 8, &dwReadByteLength );

		dwSrcPtr += dwReadByteLength;

		clmbtWork[dwWorkPtr++] = (BYTE) dwData;

		// 解凍

		for( DWORD dwCount = 1 ; dwCount < dwColorSize ; dwCount += 4 )
		{
			DWORD				dwFlags = GetBit( &clmbtSrc[dwSrcPtr], 3, &dwReadByteLength );
			DWORD				dwFlags2;

			dwSrcPtr += dwReadByteLength;

			switch( dwFlags )
			{
			case	0:

				clmbtWork[dwWorkPtr + 0] = (BYTE) dwData;
				clmbtWork[dwWorkPtr + 1] = (BYTE) dwData;
				clmbtWork[dwWorkPtr + 2] = (BYTE) dwData;
				clmbtWork[dwWorkPtr + 3] = (BYTE) dwData;

				dwWorkPtr += 4;
				continue;

			case	7:

				dwFlags2 = GetBit( &clmbtSrc[dwSrcPtr], 1, &dwReadByteLength ) + 1;
				dwSrcPtr += dwReadByteLength;
				break;

			default:

				dwFlags2 = (dwFlags + 2);
			}

			for( int j = 0 ; j < 4 ; j++ )
			{
				if( dwFlags2 == 8 )
				{
					dwData = GetBit( &clmbtSrc[dwSrcPtr], 8, &dwReadByteLength );
				}
				else
				{
					dwWork = GetBit( &clmbtSrc[dwSrcPtr], dwFlags2, &dwReadByteLength );

					if( dwWork & 1 )
					{
						dwData += (dwWork >> 1) + 1;
					}
					else
					{
						dwData -= (dwWork >> 1);
					}
				}

				dwSrcPtr += dwReadByteLength;
				clmbtWork[dwWorkPtr++] = (BYTE) dwData;
			}
		}
	}

	// RGBの合成

	BYTE*				pbtDst = &clmbtDst[dwPitch * (wHeight - 1)];
	const BYTE*			apbtWork[3];

	apbtWork[0] = &clmbtWork[adwWorkPtrForSave[0]];
	apbtWork[1] = &clmbtWork[adwWorkPtrForSave[1]];
	apbtWork[2] = &clmbtWork[adwWorkPtrForSave[2]];

	for( WORD j = 0 ; j < wHeight ; j++ )
	{
		if( j & 1 )
		{
			// 奇数ライン

			for( WORD k = 0 ; k < wWidth ; k++ )
			{
				pbtDst[0] = *apbtWork[0]++;
				pbtDst[1] = *apbtWork[1]++;
				pbtDst[2] = *apbtWork[2]++;

				pbtDst -= 3;
			}

			pbtDst -= (dwPitch - 3);
		}
		else
		{
			// 偶数ライン

			for( WORD k = 0 ; k < wWidth ; k++ )
			{
				pbtDst[0] = *apbtWork[0]++;
				pbtDst[1] = *apbtWork[1]++;
				pbtDst[2] = *apbtWork[2]++;

				pbtDst += 3;
			}

			pbtDst -= (dwPitch + 3);
		}
	}

	// 出力

	CImage				clImage;

	clImage.Init( pclArc, wWidth, wHeight, 24 );
	clImage.Write( &clmbtDst[0], dwDstSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	LZSSの解凍

BOOL	CNscr::DecodeLZSS(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->format != _T("LZSS") )
	{
		return	FALSE;
	}

	// バッファ確保

	DWORD				dwSrcSize = pstFileInfo->sizeCmp;
	DWORD				dwDstSize = pstFileInfo->sizeOrg;
	DWORD				dwDicSize = 256;

	YCMemory<BYTE>		clmbtSrc( dwSrcSize );
	YCMemory<BYTE>		clmbtDst( dwDstSize );
	YCMemory<BYTE>		clmbtDic( dwDicSize );

	ZeroMemory( &clmbtDic[0], dwDicSize );

	// 読み込み

	pclArc->Read( &clmbtSrc[0], dwSrcSize );

	// LZSS展開

	DWORD				dwSrcPtr = 0;
	DWORD				dwDstPtr = 0;
	DWORD				dwDicPtr = 239;

	btMaskForGetBit = 0;
	btSrcForGetBit = 0;

	while( dwDstPtr < dwDstSize )
	{
		DWORD				dwReadByteLength;
		DWORD				dwData;

		// 圧縮フラグの取得

		DWORD				dwFlag = GetBit( &clmbtSrc[dwSrcPtr], 1, &dwReadByteLength );

		dwSrcPtr += dwReadByteLength;

		if( dwFlag & 1 )
		{
			// 無圧縮

			dwData = GetBit( &clmbtSrc[dwSrcPtr], 8, &dwReadByteLength );

			clmbtDst[dwDstPtr++] = (BYTE) dwData;
			clmbtDic[dwDicPtr++] = (BYTE) dwData;

			dwSrcPtr += dwReadByteLength;
			dwDicPtr &= (dwDicSize - 1);
		}
		else
		{
			// 圧縮されている

			DWORD				dwBack = GetBit( &clmbtSrc[dwSrcPtr], 8, &dwReadByteLength );

			dwSrcPtr += dwReadByteLength;

			DWORD				dwLength = GetBit( &clmbtSrc[dwSrcPtr], 4, &dwReadByteLength ) + 2;

			dwSrcPtr += dwReadByteLength;

			if( (dwDstPtr + dwLength) > dwDstSize )
			{
				// 出力バッファを超えてしまう

				dwLength = (dwDstSize - dwDstPtr);
			}

			for( DWORD i = 0 ; i < dwLength ; i++ )
			{
				clmbtDst[dwDstPtr++] = clmbtDic[dwDicPtr++] = clmbtDic[dwBack++];

				dwDicPtr &= (dwDicSize - 1);
				dwBack &= (dwDicSize - 1);
			}
		}
	}

	// 出力

	CImage				clImage;

	clImage.Init( pclArc, &clmbtDst[0] );
	clImage.Write( dwDstSize );

	return	TRUE;
}
