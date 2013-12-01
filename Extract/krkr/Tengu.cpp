#include "stdafx.h"
#include "Tengu.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Check if it can be decoded

BOOL CTengu::OnCheckDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	return	CheckTpm( "CE093BB86595E62ADBCB1280CA6583EF" );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialize Decryption Process

DWORD CTengu::OnInitDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	// Decryption Key
	return 0x08;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decryption Process

DWORD CTengu::OnDecrypt(
	BYTE*				pbtTarget,						// Data to be decoded
	DWORD				dwTargetSize,					// Decoding size
	DWORD				dwOffset,						// Location of data to be decoded (offset)
	DWORD				dwDecryptKey					// Decryption Key
	)
{
	BYTE btDecryptKey = (BYTE) dwDecryptKey;

	// Decryption 
	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= btDecryptKey;
	}

	if( memcmp( pbtTarget, "\xFE\xFE\x01\xFF\xFE", 5 ) != 0 )
	{
		return	dwTargetSize;
	}

	// Temporary Copy
	YCMemory<BYTE> clmbtTemporary( dwTargetSize );
	memcpy( &clmbtTemporary[0], pbtTarget, dwTargetSize );
	ZeroMemory( pbtTarget, dwTargetSize );

	// Decryption 
	for( DWORD i = 5, j = 0 ; i < dwTargetSize ; i += 2, j += 2 )
	{
		WORD wWork1 = *(WORD*) &clmbtTemporary[i];
		WORD wWork2 = wWork1;

		wWork1 = (wWork1 & 0x5555) << 1;
		wWork2 = (wWork2 & 0xAAAA) >> 1;

		*(WORD*) &pbtTarget[j] = (wWork1 | wWork2);
	}

	// Convert to a multi-byte character
	YCStringA clsWork = (wchar_t*) pbtTarget;
	strcpy( (char*) pbtTarget, clsWork );

	return (DWORD) clsWork.GetLength();
}
