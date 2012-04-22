
#include	"stdafx.h"
#include	"Fate.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	復号可能か判定

BOOL	CFate::OnCheckDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	static const char*	apszMD5[] =
	{
		"4BE5F676CE80429B3245DA9E721FE23B",
		"7570E3AFA14F221268DCC48CAEE1277F",
	};

	for( DWORD i = 0 ; i < _countof( apszMD5 ) ; i++ )
	{
		if( CheckTpm( apszMD5[i] ) )
		{
			return	TRUE;
		}
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理の初期化

DWORD	CFate::OnInitDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	0x36;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理

DWORD	CFate::OnDecrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	DWORD				dwOffset,						// 復号対象データの位置
	DWORD				dwDecryptKey					// 復号キー
	)
{
	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= dwDecryptKey;

		if( (dwOffset + i) == 0x13 )
		{
			pbtTarget[i] ^= 0x01;
		}

		if( (dwOffset + i) == 0x2EA29 )
		{
			pbtTarget[i] ^= 0x03;
		}
	}

	return	dwTargetSize;
}
