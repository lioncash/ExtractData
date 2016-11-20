#include "StdAfx.h"
#include "Fate.h"

/// Determine if decryption is possible
///
/// @param archive Archive
///
bool CFate::OnCheckDecrypt(CArcFile* archive)
{
	static const std::array<const char*, 2> md5 = {{
		"4BE5F676CE80429B3245DA9E721FE23B",
		"7570E3AFA14F221268DCC48CAEE1277F",
	}};

	return std::any_of(md5.begin(), md5.end(), [this](const char* str) { return CheckTpm(str); });
}

/// Initialization of Decryption Routine
///
/// @param archive Archive
///
u32 CFate::OnInitDecrypt(CArcFile* archive)
{
	return 0x36;
}

/// Decryption Process
///
/// @param target      Data to be decoded
/// @param target_size Data size
/// @param offset      Location of data to be decoded
/// @param decrypt_key Decryption key
///
size_t CFate::OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key)
{
	for (size_t i = 0; i < target_size; i++)
	{
		target[i] ^= decrypt_key;

		if (offset + i == 0x13)
		{
			target[i] ^= 0x01;
		}

		if (offset + i == 0x2EA29)
		{
			target[i] ^= 0x03;
		}
	}

	return target_size;
}
