#include "stdafx.h"
#include "Tengu.h"

/// Determine if decryption is possible
///
/// @param archive Archive
///
bool CTengu::OnCheckDecrypt(CArcFile* archive)
{
	return CheckTpm("CE093BB86595E62ADBCB1280CA6583EF");
}

/// Initialization of the decryption process
///
/// @param archive Archive
///
u32 CTengu::OnInitDecrypt(CArcFile* archive)
{
	// Decryption Key
	return 0x08;
}

/// Decryption Process
///
/// @param target      Data to be decoded
/// @param target_size Data size
/// @param offset      Location of data to be decoded
/// @param decrypt_key Decryption key
///
size_t CTengu::OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key)
{
	const u8 byte_decrypt_key = static_cast<u8>(decrypt_key);

	// Decryption
	for (size_t i = 0; i < target_size; i++)
	{
		target[i] ^= byte_decrypt_key;
	}

	if (memcmp(target, "\xFE\xFE\x01\xFF\xFE", 5) != 0)
	{
		return target_size;
	}

	// Temporary Copy
	YCMemory<u8> temporary(target_size);
	memcpy(&temporary[0], target, target_size);
	memset(target, 0, target_size);

	// Decryption 
	for (size_t i = 5, j = 0; i < target_size; i += sizeof(u16), j += sizeof(u16))
	{
		u16 work1 = *(u16*)&temporary[i];
		u16 work2 = work1;

		work1 = (work1 & 0x5555) << 1;
		work2 = (work2 & 0xAAAA) >> 1;

		*(u16*)&target[j] = (work1 | work2);
	}

	// Convert to a multi-byte character
	YCStringA work = reinterpret_cast<wchar_t*>(target);
	strcpy(reinterpret_cast<char*>(target), work);

	return static_cast<size_t>(work.GetLength());
}
