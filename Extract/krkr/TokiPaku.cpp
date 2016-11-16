#include "stdafx.h"
#include "TokiPaku.h"

/// Determine if decryption is possible
///
/// @param archive Archive
///
bool CTokiPaku::OnCheckDecrypt(CArcFile* archive)
{
//	return CheckTpm("510BE09DF50DB143E90D3837D416FD0F");
	return CheckTpm("A9D18BCE341E20D25DB4DBFAAE7FBF5B");
}

/// Initialization of the decryption process
///
/// @param archive Archive
///
DWORD CTokiPaku::OnInitDecrypt(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	LPCTSTR          file_ext = PathFindExtension(file_info->name);

	if (lstrcmp(file_ext, _T(".dll")) == 0)
	{
		// Don't decrypt
		SetDecryptRequirement(false);
		return 0;
	}

	// Size to decrypt
	if ((lstrcmp(file_ext, _T(".ks")) != 0) && (lstrcmp(file_ext, _T(".tjs")) != 0) && (lstrcmp(file_ext, _T(".asd")) != 0))
	{
		SetDecryptSize(256);
	}

	// Decryption Key
	DWORD decrypt_key = 0xFFFFFFFF;
	BYTE* key = (BYTE*)&decrypt_key;
	for (int i = 0; i < lstrlen(file_ext); i++)
	{
		key[i] = ~file_ext[i];
	}

	return decrypt_key;
}

/// Decryption Process
///
/// @param target      Data to be decoded
/// @param target_size Data size
/// @param offset      Location of data to be decoded
/// @param decrypt_key Decryption key
///
DWORD CTokiPaku::OnDecrypt(BYTE* target, DWORD target_size, DWORD offset, DWORD decrypt_key)
{
	for (size_t i = 0; i < target_size; i += 4)
	{
		*(DWORD*)&target[i] ^= decrypt_key;
	}

	return target_size;
}
