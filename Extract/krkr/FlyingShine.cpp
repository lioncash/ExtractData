#include "stdafx.h"
#include "FlyingShine.h"

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
DWORD CFlyingShine::OnInitDecrypt(CArcFile* archive)
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
DWORD CFlyingShine::OnDecrypt(BYTE* target, DWORD target_size, DWORD offset, DWORD decrypt_key)
{
	BYTE decrypt_key1 = (BYTE)((decrypt_key >> 8) & 0xFF);
	BYTE decrypt_key2 = (BYTE)((decrypt_key & 0xFF) % 8);

	for (size_t i = 0; i < target_size; i++)
	{
		target[i] ^= decrypt_key1;

		// Rotate Right
		target[i] = (target[i] >> decrypt_key2) | (target[i] << (8 - decrypt_key2));
	}

	return target_size;
}
