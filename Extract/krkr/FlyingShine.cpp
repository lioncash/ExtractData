#include "StdAfx.h"
#include "Extract/krkr/FlyingShine.h"

#include "ArcFile.h"
#include "Common.h"

/// Determine if decryption is possible
///
/// @param archive Archive
///
bool CFlyingShine::OnCheckDecrypt(CArcFile* archive)
{
	return CheckTpm("2FAA3AF83558C93EC2C44F06DD727ED5");
}

/// Initialization of the decryption process
///
/// @param archive Archive
///
u32 CFlyingShine::OnInitDecrypt(CArcFile* archive)
{
	return archive->GetOpenFileInfo()->key & 0xFFFF;
}

/// Decryption Process
///
/// @param target      Data to be decoded
/// @param target_size Data size
/// @param offset      Location of data to be decoded
/// @param decrypt_key Decryption key
///
size_t CFlyingShine::OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key)
{
	const u8 decrypt_key1 = static_cast<u8>((decrypt_key >> 8) & 0xFF);
	const u8 decrypt_key2 = static_cast<u8>((decrypt_key & 0xFF) % 8);

	for (size_t i = 0; i < target_size; i++)
	{
		target[i] ^= decrypt_key1;

		// Rotate Right
		target[i] = (target[i] >> decrypt_key2) | (target[i] << (8 - decrypt_key2));
	}

	return target_size;
}
