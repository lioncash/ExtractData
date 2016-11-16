#include "stdafx.h"
#include "Hachukano.h"

/// Determine if decryption is possible
///
/// @param archive Archive
///
bool CHachukano::OnCheckDecrypt(CArcFile* archive)
{
	return archive->CheckExe(_T("hachukano.exe"));
}

/// Initialization of the decryption process
///
/// @param archive Archive
///
DWORD CHachukano::OnInitDecrypt(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	LPCTSTR file_ext = PathFindExtension(file_info->name);

	if ((lstrcmp(file_ext, _T(".dll")) == 0) || (file_info->name == _T("startup.tjs")))
	{
		// Files we don't decode
		SetDecryptRequirement(false);
		return 0;
	}

	// Size to decrypt
	if ((lstrcmp(file_ext, _T(".ks")) != 0) && (lstrcmp(file_ext, _T(".tjs")) != 0) && (lstrcmp(file_ext, _T(".asd")) != 0))
	{
		SetDecryptSize(512);
	}

	// Decryption key
	m_change_decrypt_key = 0;

	return (file_info->key ^ 0x03020100);
}

/// Decryption Process
///
/// @param target      Data to be decoded
/// @param target_size Data size
/// @param offset      Location of data to be decoded
/// @param decrypt_key Decryption key
///
DWORD CHachukano::OnDecrypt(BYTE* target, DWORD target_size, DWORD offset, DWORD decrypt_key)
{
	// Decrypt
	for (size_t i = 0; i < target_size; i += 4)
	{
		if ((i & 255) == 0)
		{
			m_change_decrypt_key = 0;
		}
		else
		{
			m_change_decrypt_key += 0x04040404;
		}

		*(DWORD*)&target[i] ^= decrypt_key ^ m_change_decrypt_key;
	}

	return target_size;
}
