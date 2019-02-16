#include "StdAfx.h"
#include "Extract/krkr/Braban.h"

#include "ArcFile.h"
#include "Common.h"

/// Determine if decryption is possible
///
/// @param archive Archive
///
bool CBraban::OnCheckDecrypt(CArcFile* archive)
{
	return CheckTpm("C42B141D42FCBCDA29850FA9E9FE3FF2");
}

/// Initialization of the decryption process
///
/// @param archive Archive
///
u32 CBraban::OnInitDecrypt(CArcFile* archive)
{
	return ((archive->GetOpenFileInfo()->key ^ 0xFF) & 0xFF) - 1;
}

/// Decryption Process
///
/// @param target      Data to be decoded
/// @param target_size Data size
/// @param offset      Location of data to be decoded
/// @param decrypt_key Decryption key
///
size_t CBraban::OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key)
{
	const u8 byte_decrypt_key = static_cast<u8>(decrypt_key);

	for (size_t i = 0; i < target_size; i++)
	{
		target[i] ^= byte_decrypt_key;
	}

	return target_size;
}
