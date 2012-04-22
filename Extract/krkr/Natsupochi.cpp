
#include	"stdafx.h"
#include	"Natsupochi.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	復号可能か判定

BOOL	CNatsupochi::OnCheckDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	CheckTpm( "B1CCCE8E4048B563B316D8BAEDF7E2B1" );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理の初期化

DWORD	CNatsupochi::OnInitDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	(pclArc->GetOpenFileInfo()->key >> 3);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理

DWORD	CNatsupochi::OnDecrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	DWORD				dwOffset,						// 復号対象データの位置
	DWORD				dwDecryptKey					// 復号キー
	)
{
	BYTE				btDecryptKey = (BYTE) (dwDecryptKey & 0xFF);

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= btDecryptKey;
	}

	return	dwTargetSize;
}
