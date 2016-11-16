#include "stdafx.h"
#include "Braban.h"

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
DWORD CBraban::OnInitDecrypt(CArcFile* archive)
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
DWORD CBraban::OnDecrypt(BYTE* target, DWORD target_size, DWORD offset, DWORD decrypt_key)
{
	BYTE byte_decrypt_key = (BYTE) decrypt_key;

	for (size_t i = 0; i < target_size; i++)
	{
		target[i] ^= byte_decrypt_key;
	}

	return target_size;
}
