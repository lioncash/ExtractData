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
DWORD CTengu::OnInitDecrypt(CArcFile* archive)
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
DWORD CTengu::OnDecrypt(BYTE* target, DWORD target_size, DWORD offset, DWORD decrypt_key)
{
	BYTE byte_decrypt_key = (BYTE)decrypt_key;

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
	YCMemory<BYTE> temporary(target_size);
	memcpy(&temporary[0], target, target_size);
	ZeroMemory(target, target_size);

	// Decryption 
	for (size_t i = 5, j = 0; i < target_size; i += 2, j += 2)
	{
		WORD work1 = *(WORD*)&temporary[i];
		WORD work2 = work1;

		work1 = (work1 & 0x5555) << 1;
		work2 = (work2 & 0xAAAA) >> 1;

		*(WORD*)&target[j] = (work1 | work2);
	}

	// Convert to a multi-byte character
	YCStringA work = (wchar_t*)target;
	strcpy((char*)target, work);

	return (DWORD)work.GetLength();
}
