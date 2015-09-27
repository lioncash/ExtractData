#include "stdafx.h"
#include "SisMiko.h"

/// Determine if decryption is possible
///
/// @param pclArc Archive
///
BOOL CSisMiko::OnCheckDecrypt(CArcFile* pclArc)
{
	return pclArc->CheckExe( _T("SisuMiko.exe") );
}

/// Initialization of the decryption process
///
/// @param pclArc Archive
///
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

/// Decryption Process
///
/// @param pbtTarget    Data to be decoded
/// @param dwTargetSize Data size
/// @param dwOffset     Location of data to be decoded
/// @param dwDecryptKey Decryption key
///
DWORD CSisMiko::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	// Decrypt
	for (DWORD i = 0; i < dwTargetSize; i += 4)
	{
		*(DWORD*)&pbtTarget[i] ^= dwDecryptKey;
	}

	return dwTargetSize;
}
