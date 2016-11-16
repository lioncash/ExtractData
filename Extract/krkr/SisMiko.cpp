#include "stdafx.h"
#include "SisMiko.h"

/// Determine if decryption is possible
///
/// @param archive Archive
///
bool CSisMiko::OnCheckDecrypt(CArcFile* archive)
{
	return archive->CheckExe( _T("SisuMiko.exe") );
}

/// Initialization of the decryption process
///
/// @param archive Archive
///
DWORD CSisMiko::OnInitDecrypt(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	LPCTSTR          file_ext = PathFindExtension(file_info->name);
	DWORD            key = archive->GetOpenFileInfo()->key;

	if ((lstrcmp(file_ext, _T(".dll")) == 0) || (file_info->name == _T("startup.tjs")))
	{
		// Files we don't decrypt
		SetDecryptRequirement(false);
		return 0;
	}

	// Size to decrypt
	if ((lstrcmp(file_ext, _T(".ks")) != 0) && (lstrcmp(file_ext, _T(".tjs")) != 0) && (lstrcmp(file_ext, _T(".asd")) != 0))
	{
		SetDecryptSize(256);
	}

	// Decryption Key
	return ~((key << 16) | (key >> 16));
}

/// Decryption Process
///
/// @param target      Data to be decoded
/// @param target_size Data size
/// @param offset      Location of data to be decoded
/// @param decrypt_key Decryption key
///
DWORD CSisMiko::OnDecrypt(BYTE* target, DWORD target_size, DWORD offset, DWORD decrypt_key)
{
	for (size_t i = 0; i < target_size; i += 4)
	{
		*(DWORD*)&target[i] ^= decrypt_key;
	}

	return target_size;
}
