#include "stdafx.h"
#include "SisMiko.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Check if it can be decrypted
//
// Parameters:
//   - pclArc - Archive

BOOL CSisMiko::OnCheckDecrypt(CArcFile* pclArc)
{
	return pclArc->CheckExe( _T("SisuMiko.exe") );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialize Decryption Process
//
// Parameters:
//   - pclArc - Archive

DWORD CSisMiko::OnInitDecrypt(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	LPCTSTR    pszFileExt = PathFindExtension(pstFileInfo->name);
	DWORD      dwKey = pclArc->GetOpenFileInfo()->key;

	if ((lstrcmp(pszFileExt, _T(".dll")) == 0) || (pstFileInfo->name == _T("startup.tjs")))
	{
		// Files we don't decrypt
		SetDecryptRequirement(FALSE);
		return 0;
	}

	// Size to decrypt
	if ((lstrcmp(pszFileExt, _T(".ks")) != 0) && (lstrcmp(pszFileExt, _T(".tjs")) != 0) && (lstrcmp(pszFileExt, _T(".asd")) != 0))
	{
		SetDecryptSize(256);
	}

	// Decryption Key
	return ~((dwKey << 16) | (dwKey >> 16));
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decryption Process
//
// Parameters:
//   - pbtTarget    - Data to be decrypted
//   - dwTargetSize - Decryption size
//   - dwOffset     - Offset of data to be decoded
//   - dwDecryptKey - Decryption key

DWORD CSisMiko::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	// Decrypt
	for( DWORD i = 0 ; i < dwTargetSize ; i += 4 )
	{
		*(DWORD*) &pbtTarget[i] ^= dwDecryptKey;
	}

	return dwTargetSize;
}
