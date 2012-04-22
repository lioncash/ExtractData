
#include	"stdafx.h"
#include	"Braban.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	復号可能か判定

BOOL	CBraban::OnCheckDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	CheckTpm( "C42B141D42FCBCDA29850FA9E9FE3FF2" );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理の初期化

DWORD	CBraban::OnInitDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	((pclArc->GetOpenFileInfo()->key ^ 0xFF) & 0xFF) - 1;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理

DWORD	CBraban::OnDecrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	DWORD				dwOffset,						// 復号対象データの位置
	DWORD				dwDecryptKey					// 復号キー
	)
{
	BYTE				btDecryptkey = (BYTE) dwDecryptKey;

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= btDecryptkey;
	}

	return	dwTargetSize;
}
