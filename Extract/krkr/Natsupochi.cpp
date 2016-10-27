#include "stdafx.h"
#include "Natsupochi.h"

/// Determine if decryption is possible
///
/// @param pclArc Archive
///
bool CNatsupochi::OnCheckDecrypt(CArcFile* pclArc)
{
	return CheckTpm("B1CCCE8E4048B563B316D8BAEDF7E2B1");
}

/// Initialization of the decryption process
///
/// @param pclArc Archive
///
DWORD CNatsupochi::OnInitDecrypt(CArcFile* pclArc)
{
	return (pclArc->GetOpenFileInfo()->key >> 3);
}

/// Decryption Process
///
/// @param pbtTarget    Data to be decoded
/// @param dwTargetSize Data size
/// @param dwOffset     Location of data to be decoded
/// @param dwDecryptKey Decryption key
///
DWORD CNatsupochi::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	BYTE btDecryptKey = (BYTE)(dwDecryptKey & 0xFF);

	for (DWORD i = 0; i < dwTargetSize; i++)
	{
		pbtTarget[i] ^= btDecryptKey;
	}

	return dwTargetSize;
}
