
#include	"stdafx.h"
#include	"../../ExtractBase.h"
#include	"../../Arc/Zlib.h"
#include	"../../Image.h"
#include	"../../Ogg.h"
#include	"../../FindFile.h"
#include	"Tlg.h"
#include	"Krkr.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	マウント

BOOL	CKrkr::Mount(
	CArcFile*			pclArc							// アーカイブ
	)
{
	DWORD				dwOffset;

	if( memcmp( pclArc->GetHed(), "XP3\r\n \n\x1A\x8B\x67\x01", 11 ) == 0 )
	{
		// XP3

		dwOffset = 0;
	}
	else if( memcmp( pclArc->GetHed(), "MZ", 2 ) == 0 )
	{
		// EXE型

		if( !FindXP3FromExecuteFile( pclArc, &dwOffset ) )
		{
			return	FALSE;
		}
	}
	else
	{
		return	FALSE;
	}

	m_pclArc = pclArc;

	// tpmのMD5値の設定

	SetMD5ForTpm( pclArc );

	// 復号可能かチェック

	if( !OnCheckDecrypt( pclArc ) )
	{
		// このアーカイブは復号できない

		return	FALSE;
	}

	// インデックスの位置取得

	INT64				n64IndexPos;

	pclArc->SeekHed( 11 + dwOffset );
	pclArc->Read( &n64IndexPos, 8 );

	pclArc->SeekCur( n64IndexPos - 19 );

	BYTE				abtWork[256];

	pclArc->Read( abtWork, sizeof( abtWork ) );

	switch( abtWork[0] )
	{
	case	0x80:

		n64IndexPos = *(INT64*) &abtWork[9];
		break;
	}

	// インデックスヘッダ読み込み

	BYTE				btCmpIndex;

	pclArc->SeekHed( n64IndexPos + dwOffset );
	pclArc->Read( &btCmpIndex, 1 );

	UINT64				u64CompIndexSize;
	UINT64				u64IndexSize;

	if( btCmpIndex )
	{
		// インデックスが圧縮されている

		pclArc->Read( &u64CompIndexSize, 8 );
	}

	pclArc->Read( &u64IndexSize, 8 );

	// バッファ確保

	YCMemory<BYTE>		clmbtIndex( u64IndexSize );
	DWORD				dwIndexPtr = 0;

	// インデックスヘッダが圧縮されている場合、解凍する

	if( btCmpIndex )
	{
		// インデックスが圧縮されている

		CZlib				clZlib;

		// バッファ確保

		YCMemory<BYTE>		clmbtCompIndex( u64CompIndexSize );

		// zlib解凍

		pclArc->Read( &clmbtCompIndex[0], u64CompIndexSize );

		clZlib.Decompress( &clmbtIndex[0], u64IndexSize, &clmbtCompIndex[0], u64CompIndexSize );
	}
	else
	{
		// 無圧縮インデックス

		pclArc->Read( &clmbtIndex[0], u64IndexSize );
	}

	// インデックスからファイル情報取得

	for( UINT64 i = 0 ; i < u64IndexSize ; )
	{
		// "File" チャンク

		FileChunk			stFileChunk;

		memcpy( stFileChunk.name, &clmbtIndex[i], 4 );

		stFileChunk.size = *(UINT64*) &clmbtIndex[i + 4];

		if( memcmp(stFileChunk.name, "File", 4 ) != 0 )
		{
			break;
		}

		i += 12;

		// "info" チャンク

		InfoChunk			stInfoChunk;

		memcpy( stInfoChunk.name, &clmbtIndex[i], 4 );

		stInfoChunk.size = *(UINT64*) &clmbtIndex[i + 4];
		stInfoChunk.protect = *(DWORD*) &clmbtIndex[i + 12];
		stInfoChunk.orgSize = *(UINT64*) &clmbtIndex[i + 16];
		stInfoChunk.arcSize = *(UINT64*) &clmbtIndex[i + 24];
		stInfoChunk.nameLen = *(WORD*) &clmbtIndex[i + 32];
		stInfoChunk.filename = (wchar_t*) &clmbtIndex[i + 34];

		if( memcmp( stInfoChunk.name, "info", 4 ) != 0 )
		{
			break;
		}

		i += 12 + stInfoChunk.size;

		// "segm" チャンク

		SegmChunk			stSegmChunk;

		memcpy( stSegmChunk.name, &clmbtIndex[i], 4 );

		stSegmChunk.size = *(UINT64*) &clmbtIndex[i + 4];

		if( memcmp( stSegmChunk.name, "segm", 4 ) != 0 )
		{
			break;
		}

		i += 12;

		SFileInfo			stFileInfo;

		UINT64				u64SegmCount = (stSegmChunk.size / 28);

		for( UINT64 j = 0 ; j < u64SegmCount ; j++ )
		{
			stSegmChunk.comp = *(DWORD*) &clmbtIndex[i];
			stSegmChunk.start = *(UINT64*) &clmbtIndex[i + 4] + dwOffset;
			stSegmChunk.orgSize = *(UINT64*) &clmbtIndex[i + 12];
			stSegmChunk.arcSize = *(UINT64*) &clmbtIndex[i + 20];

			stFileInfo.bCmps.push_back( stSegmChunk.comp );
			stFileInfo.starts.push_back( stSegmChunk.start );
			stFileInfo.sizesOrg.push_back( stSegmChunk.orgSize );
			stFileInfo.sizesCmp.push_back( stSegmChunk.arcSize );

			i += 28;
		}

		// 他にチャンクがないかチェック

		UINT64				u64Remainder = stFileChunk.size - 12 - stInfoChunk.size - 12 - stSegmChunk.size;

		if( u64Remainder > 0 )
		{
			// "adlr" チャンク

			if( memcmp( &clmbtIndex[i], "adlr", 4 ) == 0 )
			{
				AdlrChunk			stAdlrChunk;

				memcpy( stAdlrChunk.name, &clmbtIndex[i], 4 );

				stAdlrChunk.size = *(UINT64*) &clmbtIndex[i + 4];
				stAdlrChunk.key = *(DWORD*) &clmbtIndex[i + 12];

				stFileInfo.key = stAdlrChunk.key;
			}

			i += u64Remainder;
		}

		// リストビュー表示表の構造体に格納

		stFileInfo.name.Copy( stInfoChunk.filename, stInfoChunk.nameLen );
		stFileInfo.sizeOrg = stInfoChunk.orgSize;
		stFileInfo.sizeCmp = stInfoChunk.arcSize;
		stFileInfo.start = stFileInfo.starts[0];
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		if( stSegmChunk.comp )
		{
			stFileInfo.format = _T("zlib");
		}

		pclArc->AddFileInfo( stFileInfo );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デコード

BOOL	CKrkr::Decode(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( (pclArc->GetArcExten() != _T(".xp3")) && (pclArc->GetArcExten() != _T(".exe")) )
	{
		return	FALSE;
	}

	YCString			clsFileExt = PathFindExtension( pstFileInfo->name );

	clsFileExt.MakeLower();

	InitDecrypt( pclArc );

//char s[256];
//_stprintf(s, "%08X", pInfFile->key);
//MessageBox(NULL, s, "", 0);

	// バッファ確保

	DWORD				dwBufferSize = pclArc->GetBufSize();

	YCMemory<BYTE>		clmbtBuffer;

	// メモリに結合するかどうか

	BOOL				bComposeMemory = FALSE;

	if( (clsFileExt == _T(".tlg")) ||
		((clsFileExt == _T(".ogg")) && pclArc->GetOpt()->bFixOgg) ||
		(clsFileExt == _T(".bmp")) )
	{
		// TLG, OGG(CRC修正有り), BMP

		clmbtBuffer.resize( pstFileInfo->sizeOrg + 3 );
		bComposeMemory = TRUE;
	}
	else if( (m_dwDecryptKey == 0) &&
		pclArc->GetOpt()->bEasyDecrypt && (
		(clsFileExt == _T(".tjs")) ||
		(clsFileExt == _T(".ks")) ||
		(clsFileExt == _T(".asd")) ||
		(clsFileExt == _T(".txt"))) )
	{
		// tjs, ks, asd, txt

		clmbtBuffer.resize( pstFileInfo->sizeOrg + 3 );
		bComposeMemory = TRUE;
	}
	else
	{
		// その他

		clmbtBuffer.resize( dwBufferSize + 3 );
	}

	// 出力ファイル生成

	if( !bComposeMemory )
	{
		pclArc->OpenFile();
	}

	DWORD				dwBufferPtr = 0;
	DWORD				dwBufferSizeBase = dwBufferSize;
	DWORD				dwWroteSize = 0;

	for( size_t i = 0 ; i < pstFileInfo->starts.size() ; i++ )
	{
		dwBufferSize = dwBufferSizeBase;

		pclArc->SeekHed( pstFileInfo->starts[i] );

		if( pstFileInfo->bCmps[i] )
		{
			// 圧縮データ

			CZlib				clZlib;

			// バッファ確保

			DWORD				dwSrcSize = pstFileInfo->sizesCmp[i];

			YCMemory<BYTE>		clmbtSrc( dwSrcSize );

			DWORD				dwDstSize = pstFileInfo->sizesOrg[i];

			YCMemory<BYTE>		clmbtDst( dwDstSize + 3 );

			// zlib解凍

			pclArc->Read( &clmbtSrc[0], dwSrcSize );

			clZlib.Decompress( &clmbtDst[0], dwDstSize, &clmbtSrc[0], dwSrcSize );

			DWORD				dwDataSize = Decrypt( &clmbtDst[0], dwDstSize, dwWroteSize );

			if( bComposeMemory )
			{
				// バッファに結合

				memcpy( &clmbtBuffer[dwBufferPtr], &clmbtDst[0], dwDataSize );

				dwBufferPtr += dwDataSize;
			}
			else
			{
				// 出力

				pclArc->WriteFile( &clmbtDst[0], dwDataSize, dwDstSize );
			}

			dwWroteSize += dwDstSize;
		}
		else
		{
			// 無圧縮データ

			if( bComposeMemory )
			{
				// バッファに結合

				DWORD				dwDstSize = pstFileInfo->sizesOrg[i];

				pclArc->Read( &clmbtBuffer[dwBufferPtr], dwDstSize );

				DWORD				dwDataSize = Decrypt( &clmbtBuffer[dwBufferPtr], dwDstSize, dwWroteSize );

				dwBufferPtr += dwDataSize;
				dwWroteSize += dwDstSize;
			}
			else
			{
				DWORD				dwDstSize = pstFileInfo->sizesOrg[i];

				for( DWORD dwWroteSizes = 0 ; dwWroteSizes != dwDstSize ; dwWroteSizes += dwBufferSize )
				{
					// バッファサイズ調整

					pclArc->SetBufSize( &dwBufferSize, dwWroteSizes, dwDstSize );

					pclArc->Read( &clmbtBuffer[0], dwBufferSize );

					DWORD				dwDataSize = Decrypt( &clmbtBuffer[0], dwBufferSize, dwWroteSize );

					pclArc->WriteFile( &clmbtBuffer[0], dwDataSize );

					dwWroteSize += dwBufferSize;
				}
			}
		}
	}

	if( clsFileExt == _T(".tlg") )
	{
		// tlgをbmpに変換

		CTlg				clTLG;

		clTLG.Decode( pclArc, &clmbtBuffer[0] );
	}
	else if( clsFileExt == _T(".ogg") && pclArc->GetOpt()->bFixOgg )
	{
		// oggのCRCを修正

		COgg				clOGG;

		clOGG.Decode( pclArc, &clmbtBuffer[0] );
	}
	else if( clsFileExt == _T(".bmp") )
	{
		// bmp出力(png変換)

		CImage				clImage;

		clImage.Init( pclArc, &clmbtBuffer[0] );
		clImage.Write( pstFileInfo->sizeOrg );
	}
	else if( m_dwDecryptKey == 0 &&
		pclArc->GetOpt()->bEasyDecrypt && (
		(clsFileExt == _T(".tjs")) ||
		(clsFileExt == _T(".ks")) ||
		(clsFileExt == _T(".asd")) ||
		(clsFileExt == _T(".txt"))) )
	{
		// テキストファイル

		DWORD				dwDstSize = pstFileInfo->sizeOrg;

		SetDecryptRequirement( TRUE );

		m_dwDecryptKey = pclArc->InitDecryptForText( &clmbtBuffer[0], dwDstSize );

		DWORD				dwDataSize = Decrypt( &clmbtBuffer[0], dwDstSize, 0 );

		pclArc->OpenFile();
		pclArc->WriteFile( &clmbtBuffer[0], dwDataSize, dwDstSize );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	抽出

BOOL	CKrkr::Extract(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	DWORD				dwBufferSize = pclArc->GetBufSize();
	DWORD				dwBufferSizeBase = dwBufferSize;

	YCMemory<BYTE>		clmbtBuffer( dwBufferSize );

	pclArc->OpenFile();

	for( size_t i = 0 ; i < pstFileInfo->starts.size() ; i++ )
	{
		dwBufferSize = dwBufferSizeBase;

		pclArc->SeekHed( pstFileInfo->starts[i] );

		DWORD				dwDstSize = pstFileInfo->sizesOrg[i];

		for( DWORD dwWroteSizes = 0 ; dwWroteSizes != dwDstSize ; dwWroteSizes += dwBufferSize )
		{
			// バッファサイズ調整

			pclArc->SetBufSize( &dwBufferSize, dwWroteSizes, dwDstSize );

			pclArc->Read( &clmbtBuffer[0], dwBufferSize );
			pclArc->WriteFile( &clmbtBuffer[0], dwBufferSize );
		}
	}

	pclArc->CloseFile();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	アーカイブフォルダ内にあるtpmのMD5値を設定

void	CKrkr::SetMD5ForTpm(
	CArcFile*			pclArc							// アーカイブ
	)
{
	if( pclArc->CheckMD5OfSet() )
	{
		// すでに設定済み

		return;
	}

	// アーカイブのディレクトリパスの取得

	TCHAR				szBasePathToTpm[MAX_PATH];

	lstrcpy( szBasePathToTpm, pclArc->GetArcPath() );
	PathRemoveFileSpec( szBasePathToTpm );

	// tpmのファイルパスの取得

	CFindFile				clFindFile;

	std::vector<YCString>&	vtsPathToTpm = clFindFile.DoFind( szBasePathToTpm, _T("*.tpm") );

	// tpmのMD5値の設定

	CMD5				clmd5Tpm;

	for( size_t i = 0 ; i < vtsPathToTpm.size() ; i++)
	{
		pclArc->SetMD5( clmd5Tpm.Calculate( vtsPathToTpm[i] ) );
	}

	pclArc->SetMD5OfFlag( TRUE );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号可能か判定

BOOL	CKrkr::OnCheckDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	アーカイブフォルダ内にあるtpmのMD5と一致するかの確認

BOOL	CKrkr::CheckTpm(
	const char*			pszMD5							// MD5
	)
{
	// 比較

	for( size_t i = 0 ; i < m_pclArc->GetMD5().size() ; i++ )
	{
		if( memcmp( pszMD5, m_pclArc->GetMD5()[i].szABCD, 32 ) == 0 )
		{
			// 一致した

			return	TRUE;
		}
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理の初期化

void	CKrkr::InitDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	m_pclArc = pclArc;

	// 復号要求の有効化

	SetDecryptRequirement( TRUE );

	// 復号サイズの設定

	SetDecryptSize( 0 );

	// オーバーライドされた復号初期化関数を呼ぶ

	m_dwDecryptKey = OnInitDecrypt( pclArc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デフォルトで簡易復号を使用

DWORD	CKrkr::OnInitDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	DWORD				dwDecryptKey = pclArc->InitDecrypt();

	if( dwDecryptKey == 0 )
	{
		// 暗号化されていない

		SetDecryptRequirement( FALSE );
	}

	return	dwDecryptKey;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理

DWORD	CKrkr::Decrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	DWORD				dwOffset						// 復号対象データの位置
	)
{
	// 復号しないファイル

	if( !m_bDecrypt )
	{
		// 復号要求なし

		return	dwTargetSize;
	}

	DWORD				dwDecryptSize = m_dwDecryptSize;

	if( dwDecryptSize == 0 )
	{
		// 復号サイズが設定されていない

		return	OnDecrypt( pbtTarget, dwTargetSize, dwOffset, m_dwDecryptKey );
	}
	else
	{
		// 復号サイズが設定されている

		if( dwOffset >= dwDecryptSize )
		{
			// これ以上復号しない

			return	dwTargetSize;
		}

		if( dwDecryptSize > dwTargetSize )
		{
			// 決められた復号サイズがデータサイズより大きい

			dwDecryptSize = dwTargetSize;
		}

		OnDecrypt( pbtTarget, dwDecryptSize, dwOffset, m_dwDecryptKey );

		return	dwTargetSize;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デフォルトで簡易復号を使用
//
//	備考	dwDecryptKeyはOnInitDecryptの戻り値

DWORD	CKrkr::OnDecrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	DWORD				dwOffset,						// 復号対象データの位置
	DWORD				dwDecryptKey					// 復号キー
	)
{
	m_pclArc->Decrypt( pbtTarget, dwTargetSize );

	return	dwTargetSize;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号要求の設定

void	CKrkr::SetDecryptRequirement(
	BOOL				bDecrypt						// 復号要求
	)
{
	m_bDecrypt = bDecrypt;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号サイズの設定

void	CKrkr::SetDecryptSize(
	DWORD				dwDecryptSize					// 復号サイズ
	)
{
	m_dwDecryptSize = dwDecryptSize;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	実行ファイル内からXP3アーカイブへのオフセットを取得

BOOL	CKrkr::FindXP3FromExecuteFile(
	CArcFile*			pclArc,							// アーカイブ
	DWORD*				pdwOffset						// 格納先
	)
{
	if( pclArc->GetArcSize() <= 0x200000 )
	{
		// 吉里吉里の実行ファイルではない

		return	FALSE;
	}

	*pdwOffset = 16;

	pclArc->SeekHed( 16 );

	BYTE				abtBuffer[4096];
	DWORD				dwReadSize;

	do
	{
		dwReadSize = pclArc->Read( abtBuffer, sizeof(abtBuffer) );

		for( DWORD i = 0, j = 0 ; i < (dwReadSize / 16) ; i++, j += 16 )
		{
			if( memcmp( &abtBuffer[j], "XP3\r\n \n\x1A\x8B\x67\x01", 11) == 0 )
			{
				// XP3アーカイブ発見

				*pdwOffset += j;
				return	TRUE;
			}
		}

		*pdwOffset += dwReadSize;

		if( *pdwOffset >= 0x500000 )
		{
			// 検索打ち切り
			
			break;
		}

		// キャンセル確認

		if( pclArc->GetProg()->OnCancel() )
		{
			throw	-1;
		}
	}
	while( dwReadSize == sizeof(abtBuffer) );

	// XP3アーカイブなし

	*pdwOffset = 0;

	pclArc->SeekHed();

	return	FALSE;
}
