
#include	"stdafx.h"
#include	"Hachukano.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	復号可能か判定

BOOL	CHachukano::OnCheckDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	pclArc->CheckExe( _T("hachukano.exe") );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理の初期化

DWORD	CHachukano::OnInitDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();
	LPCTSTR				pszFileExt = PathFindExtension( pstFileInfo->name );

	if( (lstrcmp( pszFileExt, _T(".dll") ) == 0) || (pstFileInfo->name == _T("startup.tjs")) )
	{
		// 復号しないファイル

		SetDecryptRequirement( FALSE );
		return	0;
	}

	// 復号するサイズ

	if( (lstrcmp( pszFileExt, _T(".ks") ) != 0) && (lstrcmp( pszFileExt, _T(".tjs") ) != 0) && (lstrcmp( pszFileExt, _T(".asd") ) != 0) )
	{
		SetDecryptSize( 512 );
	}

	// 復号キー

	m_dwChangeDecryptKey = 0;

	return	(pstFileInfo->key ^ 0x03020100);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理

DWORD	CHachukano::OnDecrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	DWORD				dwOffset,						// 復号対象データの位置
	DWORD				dwDecryptKey					// 復号キー
	)
{
	// 復号

	for( DWORD i = 0 ; i < dwTargetSize ; i += 4 )
	{
		if( (i & 255) == 0 )
		{
			m_dwChangeDecryptKey = 0;
		}
		else
		{
			m_dwChangeDecryptKey += 0x04040404;
		}

		*(DWORD*) &pbtTarget[i] ^= dwDecryptKey ^ m_dwChangeDecryptKey;
	}

	return	dwTargetSize;
}
