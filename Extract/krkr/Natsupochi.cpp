#include "stdafx.h"
#include "Natsupochi.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Check if it can be decrypted
//
// Parameters:
//   - pclArc - Archive

BOOL CNatsupochi::OnCheckDecrypt(CArcFile* pclArc)
{
	return CheckTpm("B1CCCE8E4048B563B316D8BAEDF7E2B1");
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialize Decryption Process
//
// Parameters:
//   - pclArc - Archive

DWORD CNatsupochi::OnInitDecrypt(CArcFile* pclArc)
{
	return (pclArc->GetOpenFileInfo()->key >> 3);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decoding Process
//
// Parameters:
//   - pbtTarget    - Data to be decrypted
//   - dwTargetSize - Decryption size
//   - dwOffset     - Offset of data to be decoded
//   - dwDecryptKey - Decryption key

DWORD CNatsupochi::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	BYTE btDecryptKey = (BYTE) (dwDecryptKey & 0xFF);

	for( DWORD i = 0 ; i < dwTargetSize ; i++ )
	{
		pbtTarget[i] ^= btDecryptKey;
	}

	return dwTargetSize;
}
