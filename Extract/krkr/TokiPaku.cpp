
#include	"stdafx.h"
#include	"TokiPaku.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	復号可能か判定

BOOL	CTokiPaku::OnCheckDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
//	return	CheckTpm( "510BE09DF50DB143E90D3837D416FD0F" );
	return	CheckTpm( "A9D18BCE341E20D25DB4DBFAAE7FBF5B" );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理の初期化

DWORD	CTokiPaku::OnInitDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();
	LPCTSTR				pszFileExt = PathFindExtension( pstFileInfo->name );

	if( lstrcmp( pszFileExt, _T(".dll") ) == 0 )
	{
		// 復号しないファイル

		SetDecryptRequirement( FALSE );
		return	0;
	}

	// 復号するサイズ

	if( (lstrcmp( pszFileExt, _T(".ks") ) != 0) && (lstrcmp( pszFileExt, _T(".tjs") ) != 0) && (lstrcmp( pszFileExt, _T(".asd") ) != 0) )
	{
		SetDecryptSize( 256 );
	}

	// 復号キー

	DWORD				dwDecryptKey = 0xFFFFFFFF;

	BYTE*				pbtKey = &(BYTE&) dwDecryptKey;

	for( int i = 0 ; i < lstrlen( pszFileExt ) ; i++ )
	{
		pbtKey[i] = ~pszFileExt[i];
	}

	return	dwDecryptKey;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理

DWORD	CTokiPaku::OnDecrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	DWORD				dwOffset,						// 復号対象データの位置
	DWORD				dwDecryptKey					// 復号キー
	)
{
	// 復号

	for( DWORD i = 0 ; i < dwTargetSize ; i += 4 )
	{
		*(DWORD*) &pbtTarget[i] ^= dwDecryptKey;
	}

	return	dwTargetSize;
}
