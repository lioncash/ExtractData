#include	"stdafx.h"
#include	"Himesyo.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Check if it can be decrypted
//
// Parameters:
//   - pclArc - Archive

BOOL CHimesyo::OnCheckDecrypt(CArcFile* pclArc)
{
	return pclArc->CheckExe( _T("himesyo.exe") );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialize Decryption Process
//
// Parameters:
//   - pclArc - Archive

DWORD CHimesyo::OnInitDecrypt(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	LPCTSTR    pszFileExt = PathFindExtension(pstFileInfo->name);

	if ((lstrcmp(pszFileExt, _T(".dll")) == 0) || (pstFileInfo->name == _T("startup.tjs")))
	{
		// Files we don't decode
		SetDecryptRequirement(FALSE);
		return 0;
	}

	// Size to decrypt
	if ((lstrcmp(pszFileExt, _T(".ks")) != 0) && (lstrcmp(pszFileExt, _T(".tjs")) != 0) && (lstrcmp(pszFileExt, _T(".asd")) != 0))
	{
		SetDecryptSize(256);
	}

	// Decryption key
	m_dwChangeDecryptKey = 0;

	return (pstFileInfo->key ^ 0x03020100 ^ 0xFFFFFFFF);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decryption Process
//
// Parameters:
//   - pbtTarget    - Data to be decrypted
//   - dwTargetSize - Decryption size
//   - dwOffset     - Offset of data to be decoded
//   - dwDecryptKey - Decryption key

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
