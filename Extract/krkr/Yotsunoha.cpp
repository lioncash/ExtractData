
#include	"stdafx.h"
#include	"Yotsunoha.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	復号可能か判定

BOOL	CYotsunoha::OnCheckDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	CheckTpm( "AAF0A99EAF4018CB1AA5E0D9065C2239" );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理の初期化

DWORD	CYotsunoha::OnInitDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	// 復号キー

	return	(((pstFileInfo->key >> 8) & 0xFF) ^ (pstFileInfo->key & 0xFF));
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理

DWORD	CYotsunoha::OnDecrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	DWORD				dwOffset,						// 復号対象データの位置
	DWORD				dwDecryptKey					// 復号キー
	)
{
	// 復号

	BYTE				btDecryptKey = (BYTE) dwDecryptKey;

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= btDecryptKey;
	}

	return	dwTargetSize;
}
