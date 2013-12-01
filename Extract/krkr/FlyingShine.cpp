#include "stdafx.h"
#include "FlyingShine.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Check if the file can be decoded

BOOL CFlyingShine::OnCheckDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	return CheckTpm( "2FAA3AF83558C93EC2C44F06DD727ED5" );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialize decryption process

DWORD CFlyingShine::OnInitDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	return pclArc->GetOpenFileInfo()->key & 0xFFFF;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decryption

DWORD CFlyingShine::OnDecrypt(
	BYTE*				pbtTarget,						// Data to be decoded
	DWORD				dwTargetSize,					// Decoding size
	DWORD				dwOffset,						// Location of data to decrypt (offset)
	DWORD				dwDecryptKey					// Decryption Key
	)
{
	BYTE btDecryptkey1 = (BYTE) ((dwDecryptKey >> 8) & 0xFF);
	BYTE btDecryptkey2 = (BYTE) ((dwDecryptKey & 0xFF) % 8);

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= btDecryptkey1;

		// Rotate Right
		pbtTarget[i] = (pbtTarget[i] >> btDecryptkey2) | (pbtTarget[i] << (8 - btDecryptkey2));
	}

	return dwTargetSize;
}
