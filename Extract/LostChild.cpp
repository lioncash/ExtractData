
#include	"stdafx.h"
#include	"../ExtractBase.h"
#include	"../Arc/LZSS.h"
#include	"../Image.h"
#include	"LostChild.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CLostChild::Mount(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( memcmp( pclArc->GetHed(), "EPK ", 4 ) != 0 )
	{
		return	FALSE;
	}

	pclArc->SeekHed( 4 );

	// インデックスサイズ取得

	DWORD				dwIndexSize;

	pclArc->Read( &dwIndexSize, 4 );

	dwIndexSize -= 32;

	pclArc->SeekCur( 16 );

	// ファイル数取得

	DWORD				dwFiles;

	pclArc->Read( &dwFiles, 4 );

	pclArc->SeekCur( 4 );

	// インデックス取得

	YCMemory<BYTE>		clmIndex( dwIndexSize );

	pclArc->Read( &clmIndex[0], dwIndexSize );

	// ファイル名インデックス

	BYTE*				pbtFileNameIndex = &clmIndex[*(DWORD*) &clmIndex[8]] - 32;

	// 分割アーカイブファイル

	if( pclArc->GetArcName() == _T("data.epk") )
	{
		YCString			clsPathToArc = pclArc->GetArcPath();

		for( int i = 1 ; i <= 3 ; i++ )
		{
			YCString			clsArcExt;

			clsArcExt.Format( _T(".e%02d"), i );

			clsPathToArc.RenameExtension( clsArcExt );

			pclArc->Open( clsPathToArc );

			pclArc->GetProg()->ReplaceAllFileSize( pclArc->GetArcSize() );
		}

		pclArc->SetFirstArc();
	}

	// ファイル情報の取得

	DWORD				dwIndexPtr = 0;
	DWORD				dwFileNameIndexPtr = 0;

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		// ファイル名取得

		char				szFileName[_MAX_FNAME];
		DWORD				dwLength = *(DWORD*) &pbtFileNameIndex[dwFileNameIndexPtr];

		for( DWORD j = 0 ; j < dwLength ; j++ )
		{
			szFileName[j] = pbtFileNameIndex[dwFileNameIndexPtr + 4 + j] ^ 0xFF;
		}

		szFileName[dwLength] = pbtFileNameIndex[dwFileNameIndexPtr + 4 + dwLength];

		dwFileNameIndexPtr += 4 + dwLength + 1;

		// リストに追加

		SFileInfo			stFileInfo;

		stFileInfo.name = szFileName;
		stFileInfo.start = *(UINT64*) &clmIndex[dwIndexPtr + 16];
		stFileInfo.sizeCmp = *(DWORD*) &clmIndex[dwIndexPtr + 24];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.title = _T("LOST CHILD");

		dwIndexPtr += 40;

		if( stFileInfo.start > 3900000000 )
		{
			stFileInfo.start -= 3900000000;

			if( pclArc->GetArcsID() == 2 )
			{
				pclArc->SetNextArc();
			}
		}
		else if( stFileInfo.start > 2600000000 )
		{
			stFileInfo.start -= 2600000000;

			if( pclArc->GetArcsID() == 1 )
			{
				pclArc->SetNextArc();
			}
		}
		else if( stFileInfo.start > 1300000000 )
		{
			stFileInfo.start -= 1300000000;

			if( pclArc->GetArcsID() == 0 )
			{
				pclArc->SetNextArc();
			}
		}

		// ファイルサイズを足したとき1.2GBを超えたら1.2GB引いて調整

		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		if( stFileInfo.end > 1300000000 )
		{
			stFileInfo.end -= 1300000000;
		}

		pclArc->AddFileInfo( stFileInfo );
	}

	pclArc->SetFirstArc();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード

BOOL	CLostChild::Decode(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->title != _T("LOST CHILD") )
	{
		return	FALSE;
	}

	if( DecodeESUR( pclArc ) )
	{
		return	TRUE;
	}

	if( DecodeLAD( pclArc ) )
	{
		return	TRUE;
	}

	return	Extract( pclArc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ESURのデコード

BOOL	CLostChild::DecodeESUR(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->format != _T("SUR") )
	{
		return	FALSE;
	}

	DWORD				dwReadSize;

	// 読み込み

	DWORD				dwSrcSize = pstFileInfo->sizeCmp;

	YCMemory<BYTE>		clmSrc( dwSrcSize );

	dwReadSize = pclArc->Read( &clmSrc[0], dwSrcSize );

	if( dwReadSize < dwSrcSize )
	{
		pclArc->SetNextArc();
		pclArc->SeekHed();

		pclArc->Read( &clmSrc[dwReadSize], (dwSrcSize - dwReadSize) );
	}

	// ヘッダ情報の取得

	long				lWidth = *(long*) &clmSrc[8];
	long				lHeight = *(long*) &clmSrc[12];
	DWORD				dwDstSize = *(DWORD*) &clmSrc[4] - 32;
	WORD				wBpp = 32;

	// LZSS解凍用バッファの確保

	YCMemory<BYTE>		clmDst( dwDstSize );

	// LZSS解凍

	DecompLZSS( &clmDst[0], dwDstSize, &clmSrc[32], (dwSrcSize - 32), 4096, 4078, 3 );

	// 出力

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmDst[0], dwDstSize );
	clImage.Close();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	LADのデコード

BOOL	CLostChild::DecodeLAD(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->format != _T("LAD") )
	{
		return	FALSE;
	}

	DWORD				dwReadSize;

	// 読み込み

	DWORD				dwSrcSize = pstFileInfo->sizeCmp;

	YCMemory<BYTE>		clmSrc( dwSrcSize );

	dwReadSize = pclArc->Read( &clmSrc[0], dwSrcSize );

	if( dwReadSize < dwSrcSize )
	{
		pclArc->SetNextArc();
		pclArc->SeekHed();

		pclArc->Read( &clmSrc[dwReadSize], (dwSrcSize - dwReadSize) );
	}

	// ヘッダ情報の取得

	long				lWidth = *(long*) &clmSrc[8];
	long				lHeight = *(long*) &clmSrc[12];
	DWORD				dwDstSize = *(DWORD*) &clmSrc[28];
	WORD				wBpp = 8;

	// LZSS解凍用バッファの確保

	YCMemory<BYTE>		clmDst( dwDstSize );

	// LZSS解凍

	DecompLZSS( &clmDst[0], dwDstSize, &clmSrc[32], (dwSrcSize - 32), 4096, 4078, 3 );

	pclArc->OpenFile();
	pclArc->WriteFile( &clmDst[0], dwDstSize, dwSrcSize );
	pclArc->CloseFile();

	// 出力

//	CImage				clImage;

//	clImage.Init( pclArc, lWidth, lHeight, wBpp );
//	clImage.WriteReverse( &clmDst[0], dwDstSize );
//	clImage.Close();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	LZSSの解凍

BOOL	CLostChild::DecompLZSS(
	void*				pvDst,							// 格納先
	DWORD				dwDstSize,						// 格納先サイズ
	const void*			pvSrc,							// 圧縮データ
	DWORD				dwSrcSize,						// 圧縮データサイズ
	DWORD				dwDicSize,						// 辞書サイズ
	DWORD				dwDicPtr,						// 辞書初期参照位置
	DWORD				dwLengthOffset					// 一致長のオフセット
	)
{
	BYTE*				pbtDst = (BYTE*) pvDst;
	const BYTE*			pbtSrc = (const BYTE*) pvSrc;

	// 辞書バッファの確保

	YCMemory<BYTE>		clmbtDic( dwDicSize );

	ZeroMemory( &clmbtDic[0], dwDicSize );

	// 解凍

	DWORD				dwSrcPtr = 0;
	DWORD				dwDstPtr = 0;
	BYTE				btFlags;
	DWORD				dwBitCount = 0;

	while( (dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) )
	{
		if( dwBitCount == 0 )
		{
			// 8bit読み切った

			btFlags = pbtSrc[dwSrcPtr++];
			dwBitCount = 8;
		}

		if( btFlags & 1 )
		{
			// 無圧縮データ

			pbtDst[dwDstPtr] = clmbtDic[dwDicPtr] = pbtSrc[dwSrcPtr];

			dwDstPtr++;
			dwSrcPtr++;
			dwDicPtr++;

			dwDicPtr &= (dwDicSize - 1);
		}
		else
		{
			// 圧縮データ

			BYTE				btLow = pbtSrc[dwSrcPtr++];
			BYTE				btHigh = pbtSrc[dwSrcPtr++];

			DWORD				dwBack = ((btLow << 4) | (btHigh >> 4));
			DWORD				dwLength = ((btHigh & 0x0F) + dwLengthOffset);

			if( (dwDstPtr + dwLength) > dwDstSize )
			{
				// 出力バッファを超えてしまう

				dwLength = (dwDstSize - dwDstPtr);
			}

			for( DWORD j = 0 ; j < dwLength ; j++ )
			{
				pbtDst[dwDstPtr] = clmbtDic[dwDicPtr] = clmbtDic[dwBack];

				dwDstPtr++;
				dwDicPtr++;
				dwBack++;

				dwDicPtr &= (dwDicSize - 1);
				dwBack &= (dwDicSize - 1);
			}
		}

		btFlags >>= 1;
		dwBitCount--;
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	抽出

BOOL	CLostChild::Extract(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( pstFileInfo->title != _T("LOST CHILD") )
	{
		return	FALSE;
	}

	// バッファ確保

	DWORD				dwBufferSize = pclArc->GetBufSize();

	YCMemory<BYTE>		clmBuffer( dwBufferSize );

	// 出力ファイル生成

	pclArc->OpenFile();

	for( DWORD WriteSize = 0 ; WriteSize != pstFileInfo->sizeCmp ; WriteSize += dwBufferSize )
	{
		// バッファサイズ調整

		pclArc->SetBufSize( &dwBufferSize, WriteSize );

		// 読み書き

		DWORD				dwReadSize;

		dwReadSize = pclArc->Read( &clmBuffer[0], dwBufferSize );
		pclArc->WriteFile( &clmBuffer[0], dwReadSize );

		if( dwReadSize < dwBufferSize )
		{
			pclArc->SetNextArc();

			pclArc->SeekHed();
			pclArc->Read( &clmBuffer[0], (dwBufferSize - dwReadSize) );
			pclArc->WriteFile( &clmBuffer[0], (dwBufferSize - dwReadSize) );
		}
	}

	pclArc->CloseFile();

	return	TRUE;
}
