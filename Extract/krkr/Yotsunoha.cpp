#include "stdafx.h"
#include "Yotsunoha.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Check if it can be decrypted

BOOL CYotsunoha::OnCheckDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	return CheckTpm( "AAF0A99EAF4018CB1AA5E0D9065C2239" );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialize Decryption Key

DWORD CYotsunoha::OnInitDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	// Decryption key
	return (((pstFileInfo->key >> 8) & 0xFF) ^ (pstFileInfo->key & 0xFF));
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decryption Process

DWORD CYotsunoha::OnDecrypt(
	BYTE*				pbtTarget,						// Data to be decoded
	DWORD				dwTargetSize,					// Decoding Size
	DWORD				dwOffset,						// Location of data to decode (offset)
	DWORD				dwDecryptKey					// Decryption Key
	)
{
	// Decryption
	BYTE btDecryptKey = (BYTE) dwDecryptKey;

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= btDecryptKey;
	}

	return dwTargetSize;
}
