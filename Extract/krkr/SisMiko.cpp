
#include	"stdafx.h"
#include	"SisMiko.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	復号可能か判定

BOOL	CSisMiko::OnCheckDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	pclArc->CheckExe( _T("SisuMiko.exe") );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理の初期化

DWORD	CSisMiko::OnInitDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();
	LPCTSTR				pszFileExt = PathFindExtension( pstFileInfo->name );
	DWORD				dwKey = pclArc->GetOpenFileInfo()->key;

	if( (lstrcmp( pszFileExt, _T(".dll") ) == 0) || (pstFileInfo->name == _T("startup.tjs")) )
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

	return	~((dwKey << 16) | (dwKey >> 16));
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理

DWORD	CSisMiko::OnDecrypt(
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
