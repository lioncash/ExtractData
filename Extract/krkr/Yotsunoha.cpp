#include "stdafx.h"
#include "Yotsunoha.h"

/// Determine if decryption is possible
///
/// @param pclArc Archive
///
bool CYotsunoha::OnCheckDecrypt(CArcFile* pclArc)
{
	return CheckTpm("AAF0A99EAF4018CB1AA5E0D9065C2239");
}

/// Initialization of the decryption process
///
/// @param pclArc Archive
///
DWORD CYotsunoha::OnInitDecrypt(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	// Decryption key
	return (((file_info->key >> 8) & 0xFF) ^ (file_info->key & 0xFF));
}

/// Decryption Process
///
/// @param pbtTarget    Data to be decoded
/// @param dwTargetSize Data size
/// @param dwOffset     Location of data to be decoded
/// @param dwDecryptKey Decryption key
///
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
