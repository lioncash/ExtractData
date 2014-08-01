#include "stdafx.h"
#include "TokiPaku.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Check if it can be decoded
//
// Parameters:
//   - pclArc - Archive

BOOL CTokiPaku::OnCheckDecrypt(CArcFile* pclArc)
{
//	return CheckTpm( "510BE09DF50DB143E90D3837D416FD0F" );
	return CheckTpm( "A9D18BCE341E20D25DB4DBFAAE7FBF5B" );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialize Decryption Process
//
// Parameters:
//   - pclArc - Archive

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

//////////////////////////////////////////////////////////////////////////////////////////
// Decryption Process
//
// Parameters:
//   - pbtTarget    - Data to be decrypted
//   - dwTargetSize - Decryption size
//   - dwOffset     - Offset of data to be decoded
//   - dwDecryptKey - Decryption key

DWORD CTokiPaku::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	// Decrypt
	for (DWORD i = 0; i < dwTargetSize; i += 4)
	{
		*(DWORD*)&pbtTarget[i] ^= dwDecryptKey;
	}

	return dwTargetSize;
}
