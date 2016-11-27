#include "StdAfx.h"
#include "Extract/krkr/Natsupochi.h"

#include "ArcFile.h"

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
u32 CNatsupochi::OnInitDecrypt(CArcFile* archive)
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
size_t CNatsupochi::OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key)
{
	const u8 byte_decrypt_key = static_cast<u8>(decrypt_key);

	for (size_t i = 0; i < target_size; i++)
	{
		target[i] ^= byte_decrypt_key;
	}

	return target_size;
}
