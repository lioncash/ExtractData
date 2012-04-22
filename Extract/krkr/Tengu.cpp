
#include	"stdafx.h"
#include	"Tengu.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	復号可能か判定

BOOL	CTengu::OnCheckDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	return	CheckTpm( "CE093BB86595E62ADBCB1280CA6583EF" );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理の初期化

DWORD	CTengu::OnInitDecrypt(
	CArcFile*			pclArc							// アーカイブ
	)
{
	// 復号キー

	return	0x08;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	復号処理

DWORD	CTengu::OnDecrypt(
	BYTE*				pbtTarget,						// 復号対象データ
	DWORD				dwTargetSize,					// 復号サイズ
	DWORD				dwOffset,						// 復号対象データの位置
	DWORD				dwDecryptKey					// 復号キー
	)
{
	BYTE				btDecryptKey = (BYTE) dwDecryptKey;

	// 復号

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= btDecryptKey;
	}

	if( memcmp( pbtTarget, "\xFE\xFE\x01\xFF\xFE", 5 ) != 0 )
	{
		return	dwTargetSize;
	}

	// テンポラリにコピー

	YCMemory<BYTE>		clmbtTemporary( dwTargetSize );

	memcpy( &clmbtTemporary[0], pbtTarget, dwTargetSize );

	ZeroMemory( pbtTarget, dwTargetSize );

	// 復号

	for( DWORD i = 5, j = 0 ; i < dwTargetSize ; i += 2, j += 2 )
	{
		WORD				wWork1 = *(WORD*) &clmbtTemporary[i];
		WORD				wWork2 = wWork1;

		wWork1 = (wWork1 & 0x5555) << 1;
		wWork2 = (wWork2 & 0xAAAA) >> 1;

		*(WORD*) &pbtTarget[j] = (wWork1 | wWork2);
	}

	// マルチバイト文字に変換

	YCStringA			clsWork = (wchar_t*) pbtTarget;

	strcpy( (char*) pbtTarget, clsWork );

	return	(DWORD) clsWork.GetLength();
}
