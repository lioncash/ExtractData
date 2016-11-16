#include "stdafx.h"
#include "Himesyo.h"

/// Determine if decryption is possible
///
/// @param archive Archive
///
bool CHimesyo::OnCheckDecrypt(CArcFile* archive)
{
	return archive->CheckExe( _T("himesyo.exe") );
}

/// Initialization of the decryption process
///
/// @param archive Archive
///
DWORD CHimesyo::OnInitDecrypt(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	LPCTSTR          file_ext = PathFindExtension(file_info->name);

	if ((lstrcmp(file_ext, _T(".dll")) == 0) || (file_info->name == _T("startup.tjs")))
	{
		// Files we don't decode
		SetDecryptRequirement(false);
		return 0;
	}

	// Size to decrypt
	if ((lstrcmp(file_ext, _T(".ks")) != 0) && (lstrcmp(file_ext, _T(".tjs")) != 0) && (lstrcmp(file_ext, _T(".asd")) != 0))
	{
		SetDecryptSize(256);
	}

	// Decryption key
	m_change_decrypt_key = 0;

	return (file_info->key ^ 0x03020100 ^ 0xFFFFFFFF);
}

/// Decryption Process
///
/// @param target      Data to be decoded
/// @param target_size Data size
/// @param offset      Location of data to be decoded
/// @param decrypt_key Decryption key
///
DWORD CHimesyo::OnDecrypt(BYTE* target, DWORD target_size, DWORD offset, DWORD decrypt_key)
{
	// Decryption
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
