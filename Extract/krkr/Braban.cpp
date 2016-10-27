#include "stdafx.h"
#include "Braban.h"

/// Determine if decryption is possible
///
/// @param pclArc Archive
///
bool CBraban::OnCheckDecrypt(CArcFile* pclArc)
{
	return CheckTpm("C42B141D42FCBCDA29850FA9E9FE3FF2");
}

/// Initialization of the decryption process
///
/// @param pclArc Archive
///
DWORD CBraban::OnInitDecrypt(CArcFile* pclArc)
{
	return ((pclArc->GetOpenFileInfo()->key ^ 0xFF) & 0xFF) - 1;
}

/// Decryption Process
///
/// @param pbtTarget    Data to be decoded
/// @param dwTargetSize Data size
/// @param dwOffset     Location of data to be decoded
/// @param dwDecryptKey Decryption key
///
DWORD CBraban::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	BYTE btDecryptkey = (BYTE) dwDecryptKey;

	for (DWORD i = 0; i < dwTargetSize; i++)
	{
		pbtTarget[i] ^= btDecryptkey;
	}

	return dwTargetSize;
}
