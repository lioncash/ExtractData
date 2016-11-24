#include "StdAfx.h"
#include "ef_latter.h"

/// Check if files are supported
///
/// @param archive Archive
///
bool CEFLatter::IsSupported(const CArcFile* archive)
{
	// Get header
	u8 header[32];
	memcpy(header, archive->GetHed(), sizeof(header));

	// Get decryption key
	m_key = header[9];

	// Decryption
	Decrypt(header, sizeof(header));
	return memcmp(header, "ef_latter", 9) == 0;
}

/// Mount
///
/// @param archive Archive
///
bool CEFLatter::Mount(CArcFile* archive)
{
	if (!IsSupported(archive))
		return false;

	// Skip 32 bytes
	archive->SeekHed(32);

	return CPaz::Mount(archive);
}

/// Decode
///
/// @param archive Archive
///
bool CEFLatter::Decode(CArcFile* archive)
{
	if (!IsSupported(archive))
		return false;

	return CPaz::Decode(archive);
}

/// Initialize Mount Key
///
/// @param archive Archive
///
void CEFLatter::InitMountKey(CArcFile* archive)
{
	static const KeyInfo key_info[] =
	{
		{ _T("scr"),   "\x07\x81\x2B\x53\xAF\xA3\x2B\x3B\x2D\xA8\x5A\xBB\x5C\x50\x50\x8B\xF3\xCD\xC3\x23\x84\xF4\x9F\xE8\x24\xFB\x74\xA7\x46\xD9\x1B\x1B" },
		{ _T("bg"),    "\x1A\x59\x52\xD9\x1E\xE4\xDA\x31\xCF\xC6\xA0\xA7\x94\xE5\x41\xA6\xBD\x37\xBB\xDF\xD0\xED\x4B\x3E\xAB\x8D\x4E\x1A\x9B\xF6\xC5\x8B" },
		{ _T("st"),    "\x09\x55\x97\x79\xD4\xE7\x0D\x93\x02\xE1\x9C\x47\x97\x81\xB5\x50\xAE\x9A\x56\xDE\xFC\x83\xE7\x4D\xFD\x71\xBE\x14\xFD\x27\x9B\xE1" },
		{ _T("sys"),   "\xDF\x74\xCF\x44\x3A\xD5\x54\x0E\xB9\x09\x52\xE4\x0F\xB0\x88\x83\x0A\x40\x26\x06\x2E\xA0\x66\x56\x51\xC9\x16\x54\xD1\xDB\xE5\xC3" },
		{ _T("bgm"),   "\xFC\xA7\x93\x36\x69\x34\x61\x41\x0F\x10\x88\xBA\x11\x9E\x50\x3E\xC8\x5F\x91\x89\xB4\xA2\x7E\x67\x33\xA5\x5D\x2F\xA0\x07\xF7\x9A" },
		{ _T("voice"), "\x00\x95\xFE\xF8\x45\x42\x0D\xCB\xFF\x3F\x56\x01\x17\xD4\x26\x32\xD1\x2C\x70\xA7\xFE\x0D\xB8\xC4\x4D\xA2\x63\x97\x6E\xED\xE2\x27" },
		{ _T("se"),    "\xD2\x30\xE2\xEC\xBB\x90\x2A\x53\xAB\x1D\xA5\xE6\xF5\xB4\xD9\x62\xEE\xB6\x6E\x83\xFA\x6B\xDD\x54\x86\x3C\xA5\xAD\x06\x01\xF1\x11" },
		{ _T("mov"),   "\xC2\xEA\xFF\xAB\x99\xCE\xE2\x4B\x9A\xA5\xFD\xEE\x5A\xCA\x77\xCA\x6A\xF3\x71\x15\xD9\x14\x48\x37\xD3\x22\x43\xD1\xBB\x36\x72\x2F" }
	};

	SetKey(archive, key_info);
}

/// Initialize Decode Key
///
/// @param archive Archive
///
void CEFLatter::InitDecodeKey(CArcFile* archive)
{
	static const KeyInfo key_info[] =
	{
		{ _T("scr"),   "\xD3\xE0\x5D\x33\x9F\x4E\xC7\x9D\xCB\xB4\x18\xFE\xBD\xCA\x7D\x1B\x36\x45\xCD\x47\x31\xE3\x2A\x2C\xF1\x04\x5A\xE5\x28\x09\xAB\xA0" },
		{ _T("bg"),    "\x9D\xFD\xE2\x56\xE0\x70\x79\x4E\x3F\x46\x8D\x49\xC6\x68\x22\x32\x00\x2D\x7B\x0A\x47\x2C\x90\x17\x9B\x44\x0B\x61\xE1\xD9\x98\x1F" },
		{ _T("st"),    "\xA2\xCC\xD4\xF8\xB6\xF7\xC9\x2F\x91\x28\xE1\x2D\x91\x8F\x71\xC7\x75\x01\x04\x47\xC8\x26\x2F\x3A\xE7\x9D\x06\xDA\x32\x16\xF6\x1E" },
		{ _T("sys"),   "\x7A\x87\x77\x86\x8F\xB0\xA2\xC3\x9E\xCD\x85\x16\xA3\x06\xF3\xD1\x58\x23\x57\xB4\x34\x90\xED\x41\xF0\x5F\x40\xB8\x3B\x0C\x81\x1C" },
		{ _T("bgm"),   "\x73\x84\x77\x7F\x4D\xFC\x9A\x95\xF6\x0A\xE6\xCD\x50\x9A\xAF\x8F\x5E\xDA\x8D\x02\x12\x0A\x6C\xB7\x40\xC5\x23\xF5\x07\x40\x28\x58" },
		{ _T("voice"), "\x7D\xA1\x2B\xE2\x3C\x15\x3D\x2D\x10\x7C\x22\xA4\x73\x69\x05\x4A\xA0\x18\xA0\x3C\xF5\x83\xEA\x85\x22\xAE\x9B\x3B\x63\xEF\xF8\xAF" },
		{ _T("se"),    "\x83\x56\xE5\xB1\x83\x64\x38\x7D\x97\x1C\x44\xFC\x17\x34\xC2\x93\xF3\x7A\x43\x6D\x90\xF5\xCE\x9E\xCD\x12\x23\xF6\x79\x91\x2C\x51" },
		{ _T("mov"),   "\xA6\x11\x55\x3D\xED\xDC\xDE\x84\x15\xCE\xE6\x97\x44\x7B\x6E\xB3\x3F\xE5\x5F\x1E\x19\x3D\x13\xAF\x88\x2E\x55\xFF\xA1\x86\x70\xCB" }
	};

	SetKey(archive, key_info);

	SetDecryptKey2(archive);
}

/// Initialize Movie Table
///
/// @param table table
///
/// @return table size
///
u32 CEFLatter::InitMovieTable(const u8* table)
{
	u8* movie_table = GetMovieTable();

	memcpy(movie_table, table, 256);

	return 256;
}

/// Set decryption key
///
/// @param archive Archive
///
void CEFLatter::SetDecryptKey2(CArcFile* archive)
{
	// Get base archive file name
	TCHAR base_archive_name[_MAX_FNAME];
	GetBaseArcName(base_archive_name, archive->GetArcName());

	// Get base key
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	LPCTSTR file_ext = PathFindExtension(file_info->name);
	YCStringA base_key;

	if ((lstrcmpi(file_ext, _T(".png")) == 0) || (lstrcmpi(file_ext, _T(".png-")) == 0))
	{
		base_key = "957277W1";
	}
	else if (lstrcmpi(file_ext, _T(".ogg")) == 0)
	{
		base_key = "m24lX440";
	}
	else if (lstrcmpi(file_ext, _T(".sc")) == 0)
	{
		base_key = "A00U43Mj";
	}
	else if (lstrcmpi(file_ext, _T(".avi")) == 0)
	{
		base_key = "8fO1Xj6g";
	}

	// No need to decode
	if (base_key.IsEmpty())
	{
		strcpy(m_key_string, "");

		return;
	}

	// Get file name (Lower-case)
	TCHAR file_name[_MAX_FNAME];

	lstrcpy(file_name, file_info->name);

	if (lstrcmpi(base_archive_name, _T("voice")) == 0)
	{
		if (lstrcmp(PathFindExtension(file_name), _T(".ogg")) == 0)
		{
			// .ogg extension is added when you open the voice archive

			PathRemoveExtension(file_name);
		}
	}

	CharLower(file_name);

	// Create Key
	sprintf(m_key_string, "%s %08X %s", file_name, file_info->sizeOrg, base_key.GetString());

	// Create Table
	if (base_key == "8fO1Xj6g" && lstrcmpi(file_ext, _T(".avi")) == 0)
	{
		// Movie File

		// Construct RC4 Table

		const u8* const movie_table = GetMovieTable();
		u8 rc4_table[256];

		const char* const key = m_key_string;
		const size_t key_length = strlen(m_key_string);

		for (size_t i = 0; i < 256; i++)
		{
			rc4_table[i] = movie_table[i] ^ key[i % key_length];
		}

		// Initialize Table
		u8 key_table[256];

		for (size_t i = 0; i < 256; i++)
		{
			key_table[i] = static_cast<BYTE>(i);
		}

		// Construct table
		for (size_t index1 = 0, index2 = 0; index1 < 256; index1++)
		{
			index2 = (index2 + key_table[index1] + rc4_table[index1]) & 0xFF;

			std::swap(key_table[index1], key_table[index2]);
		}

		// Construct movie decoding table
		for (size_t i = 0, index1 = 0, index2 = 0; i < 65536; i++)
		{
			index1 = (index1 + 1) & 0xFF;
			index2 = (index2 + key_table[index1]) & 0xFF;

			std::swap(key_table[index1], key_table[index2]);

			const u32 work = (key_table[index1] + key_table[index2]) & 0xFF;

			m_movie_decode_table[i] = key_table[work];
		}

		strcpy(m_key_string, "");

		m_movie_decode_table_ptr = 0;
	}
	else
	{
		// Initialize Table
		for (size_t i = 0; i < 256; i++)
		{
			m_key_table[i] = static_cast<u8>(i);
		}

		// Construct Table
		const char* const key = m_key_string;
		const size_t key_length = strlen(m_key_string);

		for (size_t index1 = 0, index2 = 0; index1 < 256; index1++)
		{
			index2 = (index2 + m_key_table[index1] + key[index1 % key_length]) & 0xFF;

			std::swap(m_key_table[index1], m_key_table[index2]);
		}

		m_key_table_index1 = 0;
		m_key_table_index2 = 0;
	}
}

/// Decryption
///
/// @param target Data to be decoded
/// @param size   Decoding size
///
void CEFLatter::Decrypt(u8* target, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		target[i] ^= m_key;
	}
}

/// Decryption
///
/// @param target Data to be decoded
/// @param size   Decoding size
///
void CEFLatter::Decrypt2(u8* target, size_t size)
{
	if (strcmp(m_key_string, "") == 0)
		return;

	for (size_t i = 0; i < size; i++)
	{
		m_key_table_index1 = (m_key_table_index1 + 1) & 0xFF;
		m_key_table_index2 = (m_key_table_index2 + m_key_table[m_key_table_index1]) & 0xFF;

		std::swap(m_key_table[m_key_table_index1], m_key_table[m_key_table_index2]);

		const u32 work = (m_key_table[m_key_table_index1] + m_key_table[m_key_table_index2]) & 0xFF;

		target[i] ^= m_key_table[work];
	}
}

/// Decode Movie Data
///
/// @param target Data to be decoded
/// @param size   Decoding size
///
void CEFLatter::DecodeMovieData(u8* target, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		target[i] ^= m_movie_decode_table[m_movie_decode_table_ptr];

		m_movie_decode_table_ptr = (m_movie_decode_table_ptr + 1) & 0xFFFF;
	}
}
