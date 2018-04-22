#include "StdAfx.h"
#include "Extract/paz/HaruotoFD.h"

#include "ArcFile.h"

/// Mount
///
/// @param archive Archive
///
bool CHaruotoFD::Mount(CArcFile* archive)
{
	if (!archive->CheckExe(_T("haruotoFD.exe")))
		return false;

	return CPaz::Mount(archive);
}

/// Decode
///
/// @param archive Archive
///
bool CHaruotoFD::Decode(CArcFile* archive)
{
	if (!archive->CheckExe(_T("haruotoFD.exe")))
		return false;

	return CPaz::Decode(archive);
}

/// Initialize Mount Key
///
/// @param archive Archive
///
void CHaruotoFD::InitMountKey(CArcFile* archive)
{
	static const std::array<KeyInfo, 8> key_info{{
		{ _T("scr"),    "\xC7\xBB\x3E\xE1\x9B\x13\x01\x4E\xDD\xAE\x1E\x0E\xBD\x62\xC9\x4B\x7A\x52\x11\x8B\x61\xE3\xE3\xA6\x08\x07\x34\x53\x8B\x98\xFA\x33" },
		{ _T("bg"),     "\x7F\x74\xC8\x00\x62\x2E\xC5\xA5\x46\x05\xC2\x09\x7E\xED\x21\x41\x01\x5F\x68\xF2\x49\xB6\x23\x99\x71\xDB\xE9\x89\x4F\xF8\x37\x46" },
		{ _T("st"),     "\x47\xFC\xF2\x1D\x9C\xA9\x34\x9B\xFF\xEA\x48\xB7\x12\x3D\x73\x62\xB4\xE3\x61\x4D\xBE\xA0\xEC\x90\xFF\xBE\xFB\x02\x36\x2B\x46\xFF" },
		{ _T("sys"),    "\xC8\x57\xB0\x57\xEC\xA0\xFA\x28\x20\x10\x97\x67\x89\x1E\x5E\xD5\x1C\x44\xD3\x1D\x41\x1B\xCA\xE0\xAA\xC4\x30\x6E\x31\x5D\xA5\xE7" },
		{ _T("bgm"),    "\x6A\x47\xB9\x8E\xB3\xEF\x84\x02\x84\xEA\x54\x2A\xB5\x1C\x40\x82\x83\xA5\x51\xA2\x14\x65\x0C\xA3\x2D\xC0\x0D\x3C\xF3\x79\x90\x44" },
		{ _T("voice"),  "\x58\x51\x83\x60\x12\x31\x01\xA2\xC4\xAD\xE4\xCF\x27\x82\x34\x11\xF3\xE8\x33\xDA\x37\x77\xBE\xAE\x00\x46\xD9\x9B\xEB\x2C\x06\x1D" },
		{ _T("se"),     "\x7B\xB7\x42\x2E\xEA\xC2\x5D\x3F\x39\x4C\x6D\xE5\x32\x5B\x1A\xE8\x34\xB2\x8C\x85\x6D\x0D\xB0\x9A\x5E\xAA\x99\x7C\x4D\xDF\xC3\x3B" },
		{ _T("mov"),    "\x7C\x7E\xEB\x16\xDE\xBE\x46\xD0\xFD\x7A\xD4\xBC\xE7\x74\x8F\x2F\xD0\x67\x34\x23\x37\xA4\x6D\xBE\x3F\xFF\x12\x8D\x08\xC0\x46\x26" }
	}};

	SetKey(archive, key_info);
}

/// Initialize Decode Key
///
/// @param archive Archive
///
void CHaruotoFD::InitDecodeKey(CArcFile* archive)
{
	static const std::array<KeyInfo, 8> key_info{{
		{ _T("scr"),    "\x5B\x51\x62\x23\xA5\xDD\xC9\xE5\x24\x10\x33\x79\x5F\x3A\xF0\x34\x8C\x17\xB5\x22\xEB\xB1\x0B\xD4\x68\x83\xCA\xD2\x2D\xB6\x7B\x83" },
		{ _T("bg"),     "\xFD\x1C\x53\x3A\x02\xC2\x89\x37\xDA\xCA\xCA\x47\xA1\x4E\x84\xDF\xB1\x14\xD7\x55\xEB\xCA\xD6\x13\x2D\x91\xED\x69\xCB\x93\x1D\xD3" },
		{ _T("st"),     "\xC9\x63\xDB\x68\x86\xEE\x42\x39\xAC\xB0\x6A\x52\x18\xFE\x93\x36\x7E\x17\xF3\x76\x8B\xBF\xD9\x46\x41\x2B\xF5\xAC\x50\xF8\x71\x2B" },
		{ _T("sys"),    "\x29\xEA\xB0\x8D\x92\x3B\x62\xA3\x72\x34\xBB\xAA\x96\xD5\x7B\x21\x3C\x43\x9E\xC6\x0D\xCB\x62\x84\x5F\x26\x67\x0B\x6C\xD2\xB3\xD2" },
		{ _T("bgm"),    "\x46\x35\x46\x47\x49\x45\x16\xEC\xC6\x8A\x21\x1D\xAE\x1D\x57\x47\xF4\x7D\x2D\x43\x71\xEA\x7D\xA6\x00\x15\x8A\xB6\x92\xCC\xE2\xD1" },
		{ _T("voice"),  "\x0B\x86\xD4\xF7\x8C\x68\x4C\x4C\x01\xA6\xC2\x3B\xAF\xE3\x07\x0F\x70\x67\xB7\xAD\x7A\xD8\xF9\x42\x5C\x4B\x62\x9C\xF3\x52\xBA\xEE" },
		{ _T("se"),     "\x20\xE1\x4F\xBB\xFC\xC0\xB1\xBA\x3E\x3B\x83\x55\xAC\xF3\x28\xA2\x39\x65\x11\x84\xA9\x10\x62\xB0\x6F\xDC\xB6\x6D\x80\x8E\xBA\xB1" },
		{ _T("mov"),    "\xF0\x0B\x6D\x74\xFA\x27\xB3\xEE\x55\xBD\x0A\x78\x76\xD7\x16\xE7\x99\x9A\xD0\x07\x40\xCE\x08\x08\xF0\x9C\x0A\x97\xEB\x6C\x1F\x61" }
	}};

	SetKey(archive, key_info);
}

/// Decode Table 2
void CHaruotoFD::DecodeTable2()
{
	Table& table = GetTable();

	// Decrypt 72 byte table
	u32 value1 = 0;
	u32 value2 = 0;
	u32 value3 = 0;
	u32 value4 = 0;

	for (size_t i = 0; i < 18; i += 2)
	{
		value1 ^= table[0];

		value4 = DecodeValueByTable(value1, table) ^ value2 ^ table[1];
		value3 = DecodeValueByTable(value4, table) ^ value1 ^ table[2];

		value2 = DecodeValueByTable(value3, table) ^ value4 ^ table[3];

		for (size_t j = 4; j < 16; j += 3)
		{
			value1 = DecodeValueByTable(value2, table) ^ value3 ^ table[j + 0];
			value3 = DecodeValueByTable(value1, table) ^ value2 ^ table[j + 1];
			value2 = DecodeValueByTable(value3, table) ^ value1 ^ table[j + 2];
		}

		value1 = value2 ^ table[17];
		value2 = DecodeValueByTable(value2, table) ^ value3 ^ table[16];

		table[i + 0] = value1;
		table[i + 1] = value2;
	}

	// Decrypt 4096 byte table
	size_t table_ptr = 18;

	for (size_t i = 0; i < 512; i++)
	{
		value1 ^= table[0];

		for (size_t j = 1; j < 16; j += 3)
		{
			value3 = DecodeValueByTable(value1, table) ^ value2 ^ table[j + 0];
			value2 = DecodeValueByTable(value3, table) ^ value1 ^ table[j + 1];
			value1 = DecodeValueByTable(value2, table) ^ value3 ^ table[j + 2];
		}

		value2 = DecodeValueByTable(value1, table) ^ value2 ^ table[16];
		value1 ^= table[17];

		table[table_ptr++] = value1;
		table[table_ptr++] = value2;
	}
}

/// Decode Data
///
/// @param target Data to be decoded
/// @param size   Decoding size
///
void CHaruotoFD::DecodeData(u8* target, size_t size)
{
	const Table& table = GetTable();

	for (size_t i = 0; i < size; i += 8)
	{
		u32 value1 = *reinterpret_cast<const u32*>(&target[i]) ^ table[17];
		u32 value2 = DecodeValueByTable(value1, table) ^ *reinterpret_cast<const u32*>(&target[i + 4]) ^ table[16];

		for (size_t j = 15; j > 1; j -= 2)
		{
			value1 ^= DecodeValueByTable(value2, table) ^ table[j - 0];
			value2 ^= DecodeValueByTable(value1, table) ^ table[j - 1];
		}

		value1 ^= DecodeValueByTable(value2, table);

		*reinterpret_cast<u32*>(&target[i + 0]) = table[0] ^ value2;
		*reinterpret_cast<u32*>(&target[i + 4]) = table[1] ^ value1;
	}
}
