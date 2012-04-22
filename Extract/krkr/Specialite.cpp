
#include	"stdafx.h"
#include	"Specialite.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	復号可能か判定

BOOL	CSpecialite::OnCheckDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	CheckTpm( "F78FF15C0BD396080BCDF431AED59211" );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理の初期化

DWORD	CSpecialite::OnInitDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	0xAF;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理

DWORD	CSpecialite::OnDecrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	DWORD				dwOffset,						// 復号対象データの位置
	DWORD				dwDecryptKey					// 復号キー
	)
{
	BYTE				btDecryptKey = (BYTE) dwDecryptKey;

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= btDecryptKey;
		pbtTarget[i] += 1;
	}

	return	dwTargetSize;
}
