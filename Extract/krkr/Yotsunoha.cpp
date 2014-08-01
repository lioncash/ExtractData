#include "stdafx.h"
#include "Yotsunoha.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Check if it can be decrypted
//
// Parameters:
//   - pclArc - Archive

BOOL CYotsunoha::OnCheckDecrypt(CArcFile* pclArc)
{
	return CheckTpm("AAF0A99EAF4018CB1AA5E0D9065C2239");
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialize Decryption Key
//
// Parameters:
//   - pclArc - Archive

DWORD CYotsunoha::OnInitDecrypt(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	// Decryption key
	return (((pstFileInfo->key >> 8) & 0xFF) ^ (pstFileInfo->key & 0xFF));
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decryption Process
//
// Parameters:
//   - pbtTarget    - Data to be decrypted
//   - dwTargetSize - Decryption size
//   - dwOffset     - Offset of data to be decoded
//   - dwDecryptKey - Decryption key

DWORD CYotsunoha::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	// Decryption
	BYTE btDecryptKey = (BYTE)dwDecryptKey;

	for (DWORD i = 0; i < dwTargetSize; i++)
	{
		pbtTarget[i] ^= btDecryptKey;
	}

	return dwTargetSize;
}
