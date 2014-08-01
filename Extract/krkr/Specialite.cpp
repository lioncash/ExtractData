#include "stdafx.h"
#include "Specialite.h"

//////////////////////////////////////////////////////////////////////////////////////////
//  Check if it can be decrypted
//
// Parameters:
//   - pclArc - Archive

BOOL CSpecialite::OnCheckDecrypt(CArcFile* pclArc)
{
	return CheckTpm("F78FF15C0BD396080BCDF431AED59211");
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialize Decryption Process
//
// Parameters:
//   - pclArc - Archive

DWORD CSpecialite::OnInitDecrypt(CArcFile* pclArc)
{
	return 0xAF;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decryption Process
//
// Parameters:
//   - pbtTarget    - Data to be decrypted
//   - dwTargetSize - Decryption size
//   - dwOffset     - Offset of data to be decoded
//   - dwDecryptKey - Decryption key

DWORD CSpecialite::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	BYTE btDecryptKey = (BYTE) dwDecryptKey;

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= btDecryptKey;
		pbtTarget[i] += 1;
	}

	return dwTargetSize;
}
