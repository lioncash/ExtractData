#include "StdAfx.h"
#include "Extract/krkr/TokiPaku.h"

#include "ArcFile.h"
#include "Common.h"

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
u32 CTokiPaku::OnInitDecrypt(CArcFile* archive)
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
	if (lstrcmp(file_ext, _T(".ks"))  != 0 &&
	    lstrcmp(file_ext, _T(".tjs")) != 0 &&
	    lstrcmp(file_ext, _T(".asd")) != 0)
	{
		SetDecryptSize(256);
	}

	// Decryption Key
	u32 decrypt_key = 0xFFFFFFFF;
	u8* key = reinterpret_cast<u8*>(&decrypt_key);
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
size_t CTokiPaku::OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key)
{
	for (size_t i = 0; i < target_size; i += sizeof(u32))
	{
		*reinterpret_cast<u32*>(&target[i]) ^= decrypt_key;
	}

	return target_size;
}
