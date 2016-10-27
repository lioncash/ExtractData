#include "stdafx.h"
#include "Specialite.h"

/// Determine if decryption is possible
///
/// @param pclArc Archive
///
bool CSpecialite::OnCheckDecrypt(CArcFile* pclArc)
{
	return CheckTpm("F78FF15C0BD396080BCDF431AED59211");
}

/// Initialization of the decryption process
///
/// @param pclArc Archive
///
DWORD CSpecialite::OnInitDecrypt(CArcFile* pclArc)
{
	return 0xAF;
}

/// Decryption Process
///
/// @param pbtTarget    Data to be decoded
/// @param dwTargetSize Data size
/// @param dwOffset     Location of data to be decoded
/// @param dwDecryptKey Decryption key
///
DWORD CSpecialite::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	BYTE btDecryptKey = (BYTE)dwDecryptKey;

	for (DWORD i = 0; i < dwTargetSize; i++)
	{
		pbtTarget[i] ^= btDecryptKey;
		pbtTarget[i] += 1;
	}

	return dwTargetSize;
}
