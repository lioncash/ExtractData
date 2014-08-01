#include "stdafx.h"
#include "FlyingShine.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Check if the file can be decoded
//
// Parameters:
//   - pclArc - Archive

BOOL CFlyingShine::OnCheckDecrypt(CArcFile* pclArc)
{
	return CheckTpm("2FAA3AF83558C93EC2C44F06DD727ED5");
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialize decryption process
//
// Parameters:
//   - pclArc - Archive

DWORD CFlyingShine::OnInitDecrypt(CArcFile* pclArc)
{
	return pclArc->GetOpenFileInfo()->key & 0xFFFF;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decryption
//
// Parameters:
//   - pbtTarget    - Data to be decrypted
//   - dwTargetSize - Decryption size
//   - dwOffset     - Offset of data to be decoded
//   - dwDecryptKey - Decryption key

DWORD CFlyingShine::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	BYTE btDecryptkey1 = (BYTE)((dwDecryptKey >> 8) & 0xFF);
	BYTE btDecryptkey2 = (BYTE)((dwDecryptKey & 0xFF) % 8);

	for (DWORD i = 0; i < dwTargetSize; i++)
	{
		pbtTarget[i] ^= btDecryptkey1;

		// Rotate Right
		pbtTarget[i] = (pbtTarget[i] >> btDecryptkey2) | (pbtTarget[i] << (8 - btDecryptkey2));
	}

	return dwTargetSize;
}
