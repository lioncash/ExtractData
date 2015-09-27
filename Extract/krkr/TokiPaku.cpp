#include "stdafx.h"
#include "TokiPaku.h"

/// Determine if decryption is possible
///
/// @param pclArc Archive
///
BOOL CTokiPaku::OnCheckDecrypt(CArcFile* pclArc)
{
//	return CheckTpm( "510BE09DF50DB143E90D3837D416FD0F" );
	return CheckTpm( "A9D18BCE341E20D25DB4DBFAAE7FBF5B" );
}

/// Initialization of the decryption process
///
/// @param pclArc Archive
///
DWORD CTokiPaku::OnInitDecrypt(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	LPCTSTR    pszFileExt = PathFindExtension(pstFileInfo->name);

	if (lstrcmp(pszFileExt, _T(".dll")) == 0)
	{
		// Don't decrypt
		SetDecryptRequirement(FALSE);
		return 0;
	}

	// Size to decrypt
	if ((lstrcmp(pszFileExt, _T(".ks")) != 0) && (lstrcmp(pszFileExt, _T(".tjs")) != 0) && (lstrcmp(pszFileExt, _T(".asd")) != 0))
	{
		SetDecryptSize(256);
	}

	// Decryption Key
	DWORD dwDecryptKey = 0xFFFFFFFF;
	BYTE* pbtKey = &(BYTE&)dwDecryptKey;
	for (int i = 0; i < lstrlen(pszFileExt); i++)
	{
		pbtKey[i] = ~pszFileExt[i];
	}

	return dwDecryptKey;
}

/// Decryption Process
///
/// @param pbtTarget    Data to be decoded
/// @param dwTargetSize Data size
/// @param dwOffset     Location of data to be decoded
/// @param dwDecryptKey Decryption key
///
DWORD CTokiPaku::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	// Decrypt
	for (DWORD i = 0; i < dwTargetSize; i += 4)
	{
		*(DWORD*)&pbtTarget[i] ^= dwDecryptKey;
	}

	return dwTargetSize;
}
