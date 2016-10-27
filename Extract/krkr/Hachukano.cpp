#include "stdafx.h"
#include "Hachukano.h"

/// Determine if decryption is possible
///
/// @param pclArc Archive
///
bool CHachukano::OnCheckDecrypt(CArcFile* pclArc)
{
	return pclArc->CheckExe(_T("hachukano.exe"));
}

/// Initialization of the decryption process
///
/// @param pclArc Archive
///
DWORD CHachukano::OnInitDecrypt(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	LPCTSTR pszFileExt = PathFindExtension(pstFileInfo->name);

	if ((lstrcmp(pszFileExt, _T(".dll")) == 0) || (pstFileInfo->name == _T("startup.tjs")))
	{
		// Files we don't decode
		SetDecryptRequirement(false);
		return 0;
	}

	// Size to decrypt
	if ((lstrcmp(pszFileExt, _T(".ks")) != 0) && (lstrcmp(pszFileExt, _T(".tjs")) != 0) && (lstrcmp(pszFileExt, _T(".asd")) != 0))
	{
		SetDecryptSize(512);
	}

	// Decryption key
	m_dwChangeDecryptKey = 0;

	return (pstFileInfo->key ^ 0x03020100);
}

/// Decryption Process
///
/// @param pbtTarget    Data to be decoded
/// @param dwTargetSize Data size
/// @param dwOffset     Location of data to be decoded
/// @param dwDecryptKey Decryption key
///
DWORD CHachukano::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	// Decrypt
	for (DWORD i = 0; i < dwTargetSize; i += 4)
	{
		if ((i & 255) == 0)
		{
			m_dwChangeDecryptKey = 0;
		}
		else
		{
			m_dwChangeDecryptKey += 0x04040404;
		}

		*(DWORD*)&pbtTarget[i] ^= dwDecryptKey ^ m_dwChangeDecryptKey;
	}

	return dwTargetSize;
}
