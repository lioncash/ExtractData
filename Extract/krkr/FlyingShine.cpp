
#include	"stdafx.h"
#include	"FlyingShine.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	復号可能か判定

BOOL	CFlyingShine::OnCheckDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	CheckTpm( "2FAA3AF83558C93EC2C44F06DD727ED5" );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理の初期化

DWORD	CFlyingShine::OnInitDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	pclArc->GetOpenFileInfo()->key & 0xFFFF;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理

DWORD	CFlyingShine::OnDecrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	DWORD				dwOffset,						// 復号対象データの位置
	DWORD				dwDecryptKey					// 復号キー
	)
{
	BYTE				btDecryptkey1 = (BYTE) ((dwDecryptKey >> 8) & 0xFF);
	BYTE				btDecryptkey2 = (BYTE) ((dwDecryptKey & 0xFF) % 8);

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= btDecryptkey1;

		// 右ローテート

		pbtTarget[i] = (pbtTarget[i] >> btDecryptkey2) | (pbtTarget[i] << (8 - btDecryptkey2));
	}

	return	dwTargetSize;
}
