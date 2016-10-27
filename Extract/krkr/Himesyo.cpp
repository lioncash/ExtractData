#include "stdafx.h"
#include "Himesyo.h"

/// Determine if decryption is possible
///
/// @param pclArc Archive
///
bool CHimesyo::OnCheckDecrypt(CArcFile* pclArc)
{
	return pclArc->CheckExe( _T("himesyo.exe") );
}

/// Initialization of the decryption process
///
/// @param pclArc Archive
///
DWORD CHimesyo::OnInitDecrypt(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();
	LPCTSTR          pszFileExt = PathFindExtension(file_info->name);

	if ((lstrcmp(pszFileExt, _T(".dll")) == 0) || (file_info->name == _T("startup.tjs")))
	{
		// Files we don't decode
		SetDecryptRequirement(false);
		return 0;
	}

	// Size to decrypt
	if ((lstrcmp(pszFileExt, _T(".ks")) != 0) && (lstrcmp(pszFileExt, _T(".tjs")) != 0) && (lstrcmp(pszFileExt, _T(".asd")) != 0))
	{
		SetDecryptSize(256);
	}

	// Decryption key
	m_dwChangeDecryptKey = 0;

	return (file_info->key ^ 0x03020100 ^ 0xFFFFFFFF);
}

/// Decryption Process
///
/// @param pbtTarget    Data to be decoded
/// @param dwTargetSize Data size
/// @param dwOffset     Location of data to be decoded
/// @param dwDecryptKey Decryption key
///
DWORD CHimesyo::OnDecrypt(BYTE* pbtTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey)
{
	// Decryption
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
