#include "stdafx.h"
#include "Natsupochi.h"

/// Determine if decryption is possible
///
/// @param archive Archive
///
bool CNatsupochi::OnCheckDecrypt(CArcFile* archive)
{
	return CheckTpm("B1CCCE8E4048B563B316D8BAEDF7E2B1");
}

/// Initialization of the decryption process
///
/// @param archive Archive
///
DWORD CNatsupochi::OnInitDecrypt(CArcFile* archive)
{
	return archive->GetOpenFileInfo()->key >> 3;
}

/// Decryption Process
///
/// @param target      Data to be decoded
/// @param target_size Data size
/// @param offset      Location of data to be decoded
/// @param decrypt_key Decryption key
///
DWORD CNatsupochi::OnDecrypt(BYTE* target, DWORD target_size, DWORD offset, DWORD decrypt_key)
{
	BYTE byte_decrypt_key = (BYTE)(decrypt_key & 0xFF);

	for (size_t i = 0; i < target_size; i++)
	{
		target[i] ^= byte_decrypt_key;
	}

	return target_size;
}
