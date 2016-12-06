#include "StdAfx.h"
#include "Extract/cpz/Cpz.h"

#include "ArcFile.h"
#include "Extract/cpz/PB2A.h"
#include "Extract/cpz/PB3B.h"
#include "Image.h"
#include "MD5.h"

/// Mounting
///
/// @param archive Archive
///
bool CCpz::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".cpz"))
		return false;

	if (MountCpz1(archive))
		return true;

	if (MountCpz2(archive))
		return true;

	if (MountCpz3(archive))
		return true;

	if (MountCpz5(archive))
		return true;

	return false;
}

/// CPZ1 Mounting
///
/// @param archive Archive
///
bool CCpz::MountCpz1(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "CPZ1", 4) != 0)
		return false;

	// Read header
	BYTE header[16];
	archive->Read(header, sizeof(header));
	const DWORD files = *(DWORD*)&header[4];
	const DWORD index_size = *(DWORD*)&header[8];

	// Get index
	YCMemory<BYTE> index(index_size);
	DWORD          index_ptr = 0;
	archive->Read(&index[0], index_size);

	// Decrypt the index
	Decrypt1(&index[0], index_size);
	const DWORD offset = (index_size + 16);

	for (DWORD i = 0; i < files; i++)
	{
		TCHAR file_name[256];
		lstrcpy(file_name, (LPCTSTR)&index[index_ptr + 24]);

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.sizeCmp = *(DWORD*)&index[index_ptr + 4];
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.start = *(DWORD*)&index[index_ptr + 8] + offset;
		file_info.end = file_info.start + file_info.sizeCmp;
		archive->AddFileInfo(file_info);

		index_ptr += *(DWORD*)&index[index_ptr + 0];
	}

	return true;
}

/// CPZ2 Mounting
///
/// @param archive Archive
///
bool CCpz::MountCpz2(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "CPZ2", 4) != 0)
		return false;

	// Read header
	BYTE header[20];
	archive->Read(header, sizeof(header));

	const DWORD files = *(DWORD*)&header[4] ^ 0xE47C59F3;
	const DWORD index_size = *(DWORD*)&header[8] ^ 0x3F71DE2A;
	const DWORD key = *(DWORD*)&header[16] ^ 0x77777777 ^ 0x37A9F45B;

	// Get index
	YCMemory<BYTE> index(index_size);
	DWORD index_ptr = 0;
	archive->Read(&index[0], index_size);

	// Decrypt index
	Decrypt2(&index[0], index_size, key);
	const DWORD offset = index_size + 20;

	for (DWORD i = 0; i < files; i++)
	{
		TCHAR file_name[256];
		lstrcpy(file_name, (LPCTSTR)&index[index_ptr + 24]);

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.sizeCmp = *(DWORD*)&index[index_ptr + 4];
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.start = *(DWORD*)&index[index_ptr + 8] + offset;
		file_info.end = file_info.start + file_info.sizeCmp;
		file_info.key = *(DWORD*)&index[index_ptr + 20] ^ 0x796C3AFD;
		archive->AddFileInfo(file_info);

		index_ptr += *(DWORD*)&index[index_ptr + 0];
	}

	return true;
}

/// CPZ3 Mounting
///
/// @param archive Archive
///
bool CCpz::MountCpz3(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "CPZ3", 4) != 0)
		return false;

	// Read header
	BYTE header[20];
	archive->Read(header, sizeof(header));

	const DWORD files = *(DWORD*)&header[4] ^ 0x5E9C4F37;
	const DWORD index_size = *(DWORD*)&header[8] ^ 0xF32AED17;
	const DWORD key = *(DWORD*)&header[16] ^ 0xDDDDDDDD ^ 0x7BF4A539;

	// Get index
	YCMemory<BYTE> index(index_size);
	DWORD index_ptr = 0;
	archive->Read(&index[0], index_size);

	// Decrypt index
	Decrypt3(&index[0], index_size, key);
	const DWORD offset = (index_size + 20);

	for (DWORD i = 0; i < files; i++)
	{
		TCHAR file_name[_MAX_FNAME];
		lstrcpy(file_name, (LPCTSTR)&index[index_ptr + 24]);

		// ファイル数が多いので、フォルダ名を付けてフォルダ分割できるように変更
		// プリミティブリンク用の処理なので、他タイトルで不具合出るかも
		YCString dir_name;
		LPTSTR   dir_name_end_pos = _tcschr(file_name, _T('-'));

		if (dir_name_end_pos != nullptr)
		{
			dir_name.Append(file_name, (dir_name_end_pos + 3 - file_name));
			dir_name += _T("\\");
		}

		// Add to listview
		SFileInfo file_info;
		file_info.name = dir_name + file_name;
		file_info.sizeCmp = *(DWORD*)&index[index_ptr + 4];
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.start = *(DWORD*)&index[index_ptr + 8] + offset;
		file_info.end = file_info.start + file_info.sizeCmp;
		file_info.key = *(DWORD*)&index[index_ptr + 20] ^ 0xC7F5DA63;
		archive->AddFileInfo(file_info);

		index_ptr += *(DWORD*)&index[index_ptr + 0];
	}

	return true;
}

/// CPZ5 Mounting
///
/// @param archive Archive
///
bool CCpz::MountCpz5(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "CPZ5", 4) != 0)
		return false;

	// Get header
	auto* header = reinterpret_cast<SCPZ5Header*>(archive->GetHeader());
	archive->SeekCur(sizeof(SCPZ5Header));

	// Decrypt header
	header->dwDirs ^= 0xFE3A53D9;
	header->dwTotalDirIndexSize ^= 0x37F298E7;
	header->dwTotalFileIndexSize ^= 0x7A6F3A2C;
	header->adwMD5[0] ^= 0x43DE7C19;
	header->adwMD5[1] ^= 0xCC65F415;
	header->adwMD5[2] ^= 0xD016A93C;
	header->adwMD5[3] ^= 0x97A3BA9A;
	header->dwIndexKey ^= 0xAE7D39BF;
	header->adwUnKnown2[0] ^= 0xFB73A955;
	header->adwUnKnown2[1] ^= 0x37ACF831;

	// Prepare MD5 data
	u32 md5_data[16];
	md5_data[0] = header->adwMD5[0];
	md5_data[1] = header->adwMD5[1];
	md5_data[2] = header->adwMD5[2];
	md5_data[3] = header->adwMD5[3];

	// Add padding
	CMD5 md5;
	md5.AppendPadding(md5_data, 16, 48);

	// Set initial values
	const u32 init_md5[4] = {
		0xC74A2B01,
		0xE7C8AB8F,
		0xD8BEDC4E,
		0x7302A4C5
	};

	// Calculate MD5
	const SMD5 md5_result = md5.Calculate(md5_data, sizeof(md5_data), init_md5);
	header->adwMD5[0] = md5_result.adwABCD[3];
	header->adwMD5[1] = md5_result.adwABCD[1];
	header->adwMD5[2] = md5_result.adwABCD[2];
	header->adwMD5[3] = md5_result.adwABCD[0];

	// Read index
	DWORD          index_size = (header->dwTotalDirIndexSize + header->dwTotalFileIndexSize);
	YCMemory<BYTE> index(index_size);
	archive->Read(&index[0], index_size);

	// Decode entire directory index
	Decrypt5(&index[0], index_size, (header->dwIndexKey ^ 0x3795B39A));
	const BYTE* decryption_table = InitDecryptWithTable5(header->dwIndexKey, header->adwMD5[1]);
	DecryptWithTable5(&index[0], header->dwTotalDirIndexSize, decryption_table, 0x3A);

	// Set key
	DWORD key[4];
	key[0] = header->adwMD5[0] ^ (header->dwIndexKey + 0x76A3BF29);
	key[1] = header->adwMD5[1] ^ header->dwIndexKey;
	key[2] = header->adwMD5[2] ^ (header->dwIndexKey + 0x10000000);
	key[3] = header->adwMD5[3] ^ header->dwIndexKey;

	// Decrypt the total directory index
	DWORD key_ptr = 0;
	DWORD seed = 0x76548AEF;
	BYTE* work_ptr = &index[0];

	for (DWORD i = 0; i < (header->dwTotalDirIndexSize >> 2); i++)
	{
		const DWORD work = (*(DWORD*)work_ptr ^ key[key_ptr++]) - 0x4A91C262;

		*(DWORD*)work_ptr = _lrotl(work, 3) - seed;

		key_ptr &= 3;
		work_ptr += 4;
		seed += 0x10FB562A;
	}

	for (DWORD i = 0; i < (header->dwTotalDirIndexSize & 3); i++)
	{
		*work_ptr++ = ((key[key_ptr++] >> 6) ^ *work_ptr) + 0x37;

		key_ptr &= 3;
	}

	// Initialize decryption table
	decryption_table = InitDecryptWithTable5(header->dwIndexKey, header->adwMD5[2]);

	// Get file information
	BYTE* current_dir_index_ptr = &index[0];

	for (DWORD i = 0; i < header->dwDirs; i++)
	{
		BYTE* next_dir_index_ptr = current_dir_index_ptr + *(DWORD*)&current_dir_index_ptr[0];
		const DWORD current_file_index_offset = *(DWORD*)&current_dir_index_ptr[8];
		DWORD next_file_index_offset;

		if ((i + 1) >= header->dwDirs)
		{
			// The last directory

			next_file_index_offset = header->dwTotalFileIndexSize;
		}
		else
		{
			next_file_index_offset = *(DWORD*)&next_dir_index_ptr[8];
		}

		// Decrypt the current file index
		const DWORD current_file_index_size = (next_file_index_offset - current_file_index_offset);
		BYTE* current_file_index = &index[header->dwTotalDirIndexSize + current_file_index_offset];
		DecryptWithTable5(current_file_index, current_file_index_size, decryption_table, 0x7E);

		// Set key
		const DWORD entry_key = *(DWORD*)&current_dir_index_ptr[12];
		key[0] = header->adwMD5[0] ^ entry_key;
		key[1] = header->adwMD5[1] ^ (entry_key + 0x112233);
		key[2] = header->adwMD5[2] ^ entry_key;
		key[3] = header->adwMD5[3] ^ (entry_key + 0x34258765);

		// Decrypt current file index
		seed = 0x2A65CB4E;
		work_ptr = current_file_index;
		key_ptr = 0;

		for (DWORD j = 0; (j < current_file_index_size >> 2); j++)
		{
			*(DWORD*)work_ptr = _lrotl((*(DWORD*)work_ptr ^ key[key_ptr++]) - seed, 2) + 0x37A19E8B;

			key_ptr &= 3;
			work_ptr += 4;
			seed -= 0x139FA9B;
		}

		for (DWORD j = 0; j < (current_file_index_size & 3); j++)
		{
			*work_ptr++ = (*work_ptr ^ (BYTE)(key[key_ptr++] >> 4)) + 5;

			key_ptr &= 3;
		}

		// Get file information
		BYTE* file_entry = current_file_index;

		for (DWORD j = 0; j < *(DWORD*)&current_dir_index_ptr[4]; j++)
		{
			// Get file name
			TCHAR file_name[_MAX_FNAME];
			if (strcmp((char*)&current_dir_index_ptr[16], "root") == 0)
			{
				_stprintf(file_name, _T("%s"), &file_entry[24]);
			}
			else
			{
				_stprintf(file_name, _T("%s\\%s"), &current_dir_index_ptr[16], &file_entry[24]);
			}

			// Additional file information
			SFileInfo file_info;
			file_info.name = file_name;
			file_info.start = *(UINT64*)&file_entry[4] + sizeof(SCPZ5Header)+index_size;
			file_info.sizeCmp = *(DWORD*)&file_entry[12];
			file_info.sizeOrg = file_info.sizeCmp;
			file_info.end = file_info.start + file_info.sizeCmp;
			file_info.key = (header->dwIndexKey ^ (*(DWORD*)&current_dir_index_ptr[12] + *(DWORD*)&file_entry[20])) + header->dwDirs + 0xA3D61785;
			archive->AddFileInfo(file_info);

			// Go to the next file entry
			file_entry += *(DWORD*)&file_entry[0];
		}

		// Go to the next directory entry
		current_dir_index_ptr = next_dir_index_ptr;
	}

	return true;
}

/// Decoding
///
/// @param archive Archive
///
bool CCpz::Decode(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".cpz"))
		return false;

	if (DecodeCpz1(archive))
		return true;

	if (DecodeCpz2(archive))
		return true;

	if (DecodeCpz3(archive))
		return true;

	if (DecodeCpz5(archive))
		return true;

	return false;
}

/// CPZ1 Decoding
///
/// @param archive Archive
///
bool CCpz::DecodeCpz1(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "CPZ1", 4) != 0)
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Read CPZ1
	DWORD          src_size = file_info->sizeCmp;
	YCMemory<BYTE> src(src_size);
	archive->Read(&src[0], src_size);

	// Decryption
	Decrypt1(&src[0], src_size);

	// Output
	if (file_info->format == _T("PB2"))
	{
		CPB2A pb2a;
		pb2a.Decode(archive, &src[0], src_size);
	}
	else if (file_info->format == _T("MSK"))
	{
		const long  width = *(long*)&src[8];
		const long  height = *(long*)&src[12];
		const DWORD dst_size = width * height;

		CImage image;
		image.Init(archive, width, height, 8);
		image.WriteReverse(&src[16], dst_size);
	}
	else
	{
		archive->OpenFile();
		archive->WriteFile(&src[0], src_size);
	}

	return true;
}

/// CPZ2 Decoding
///
/// @param archive Archive
///
bool CCpz::DecodeCpz2(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "CPZ2", 4) != 0)
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Read CPZ2
	DWORD          src_size = file_info->sizeCmp;
	YCMemory<BYTE> src(src_size);
	archive->Read(&src[0], src_size);

	// Decryption
	Decrypt2(&src[0], src_size, file_info->key);

	// Output
	if (file_info->format == _T("PB2"))
	{
		CPB2A pb2a;
		pb2a.Decode(archive, &src[0], src_size);
	}
	else if (file_info->format == _T("MSK"))
	{
		const long  width = *(long*)&src[8];
		const long  height = *(long*)&src[12];
		const DWORD dst_size = width * height;

		CImage image;
		image.Init(archive, width, height, 8);
		image.WriteReverse(&src[16], dst_size);
	}
	else
	{
		archive->OpenFile();
		archive->WriteFile(&src[0], src_size);
	}

	return true;
}

/// CPZ3 Decoding
///
/// @param archive Archive
///
bool CCpz::DecodeCpz3(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "CPZ3", 4) != 0)
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Read CPZ3
	DWORD          src_size = file_info->sizeCmp;
	YCMemory<BYTE> src(src_size);
	archive->Read(&src[0], src_size);

	// Decryption
	Decrypt3(&src[0], src_size, file_info->key);

	if (file_info->format == _T("PB3"))
	{
		CPB3B pb3b;
		pb3b.Decode(archive, &src[0], src_size, OnDecrypt3FromPB3B);
	}
	else
	{
		archive->OpenFile();
		archive->WriteFile(&src[0], src_size);
	}

	return true;
}

/// CPZ5 Decoding
///
/// @param archive Archive
///
bool CCpz::DecodeCpz5(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "CPZ5", 4) != 0)
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();
	const auto* cpz5_header = reinterpret_cast<const SCPZ5Header*>(archive->GetHeader());

	// Read CPZ5
	DWORD          src_size = file_info->sizeCmp;
	YCMemory<BYTE> src(src_size);
	archive->Read(&src[0], src_size);

	// Decryption
	const BYTE* table = InitDecryptWithTable5(cpz5_header->adwMD5[3], cpz5_header->dwIndexKey);
	DecryptOfData5(&src[0], src_size, table, cpz5_header->adwMD5, file_info->key);

	if (file_info->format == _T("PB3"))
	{
		CPB3B pb3b;
		pb3b.Decode(archive, &src[0], src_size, OnDecrypt5FromPB3B);
	}
	else
	{
		archive->OpenFile();
		archive->WriteFile(&src[0], src_size);
	}

	return true;
}

/// Decryption function 1
///
/// @param target Decoded data
/// @param size   Decoding size
///
void CCpz::Decrypt1(BYTE* target, DWORD size)
{
	static const BYTE crypt[] = "掴乱跨虎規日壅諺庫誤絞豪股誇砧後口糊己交\x8B\xFE謙倖規弭胡綿戸湖候誇騎";

	for (size_t i = 0; i < size; i++)
	{
		target[i] = (target[i] ^ crypt[i & 63]) - 0x6C;
	}
}

/// Decryption function 2
///
/// @param target Decoded data
/// @param size   Decoding size
/// @param key    Key
///
void CCpz::Decrypt2(BYTE* target, DWORD size, DWORD key)
{
	static const DWORD crypt[] =
	{
		0x3A68CDBF, 0xD3C3A711, 0x8414876E, 0x657BEFDB,
		0xCDD7C125, 0x09328580, 0x288FFEDD, 0x99EBF13A,
		0x5A471F95, 0x1EA3F4F1, 0xF4FF524E, 0xD358E8A9,
		0xC5B71015, 0xA913046F, 0x2D6FD2BD, 0x68C8BE19
	};

	DWORD shift = key ^ 0x50000000;
	for (int i = 0; i < 7; i++)
	{
		shift = (shift >> 4) ^ key;
	}
	shift = (shift & 0x0F) + 8;

	DWORD table_ptr = 0;
	for (DWORD i = 0; i < (size >> 2); i++)
	{
		const DWORD work = ((crypt[table_ptr++] + key) ^ *(DWORD*)target) - 0x15C3E7;

		*(DWORD*)target = (work << (32 - shift)) | (work >> shift);

		table_ptr &= 0x0F;
		target += 4;
	}

	for (DWORD i = 0, j = 0; i < (size & 3); i++, j += 4)
	{
		*target++ = (((crypt[table_ptr++] + key) >> j) ^ *target) + 0x37;

		table_ptr &= 0x0F;
	}
}

/// Decryption function 3
///
/// @param target Decoded data
/// @param size   Decoding size
/// @param key    Key
///
void CCpz::Decrypt3(BYTE* target, DWORD size, DWORD key)
{
	static const DWORD crypt[] =
	{
		0x4D0D4A5E, 0xB3ABF3E1, 0x3C37336D, 0x86C3F5F3,
		0x7D4F9B89, 0x58D7DE11, 0x6367778D, 0xA5F34629,
		0x067FA4B5, 0xED0AE742, 0xB19450CC, 0xE7204A5A,
		0xD9AF04F5, 0x5D3B687F, 0xC1C7A6FD, 0xFC502289
	};

	DWORD table[16];
	for (int i = 0; i < 16; i++)
	{
		table[i] = crypt[i] + key;
	}

	DWORD shift = key;
	for (int i = 0; i < 7; i++)
	{
		shift = (shift >> 4) ^ key;
	}

	shift = ((shift ^ 0xFFFFFFFD) & 0x0F) + 8;

	DWORD table_ptr = 3;
	for (DWORD i = 0; i < (size >> 2); i++)
	{
		const DWORD work = (table[table_ptr++] ^ *(DWORD*)target) + 0x6E58A5C2;

		*(DWORD*)target = (work << shift) | (work >> (32 - shift));

		table_ptr &= 0x0F;
		target += 4;
	}

	for (DWORD i = (size & 3); i > 0; i--)
	{
		*target++ = ((table[table_ptr++] >> (i * 4)) ^ *target) + 0x52;

		table_ptr &= 0x0F;
	}
}

/// Decryption function 5
///
/// @param target Decoded data
/// @param size   Decoding size
/// @param key    Key
///
void CCpz::Decrypt5(BYTE* target, DWORD size, DWORD key)
{
	static const DWORD crypt[] =
	{
		0xCD90F089, 0xE982B782, 0xA282AB88, 0xCD82718E, 0x52838A83, 0xA882AA82, 0x7592648E, 0xB582AB82,
		0xE182BF82, 0xDC82A282, 0x4281B782, 0xED82F48E, 0xBF82EA82, 0xA282E182, 0xB782DC82, 0x6081E682,
		0xC6824181, 0xA482A282, 0xE082A982, 0xF48EA482, 0xBF82C182, 0xA282E182, 0xB582DC82, 0xF481BD82
	};

	DWORD table[24];
	for (DWORD i = 0; i < 24; i++)
	{
		table[i] = (crypt[i] - key);
	}

	DWORD shift = key;
	for (DWORD i = 0; i < 3; i++)
	{
		shift = (shift >> 8) ^ key;
	}
	shift = ((shift ^ 0xFFFFFFFB) & 0x0F) + 7;

	DWORD table_ptr = 5;
	for (DWORD i = 0; i < (size >> 2); i++)
	{
		const DWORD work = (table[table_ptr++] ^ *(DWORD*)target) + 0x784C5962;

		*(DWORD*)target = _lrotr(work, shift) + 0x01010101;

		table_ptr %= 24;
		target += 4;
	}

	for (DWORD i = (size & 3); i > 0; i--)
	{
		*target++ = ((table[table_ptr++] >> (i * 4)) ^ *target) - 0x79;

		table_ptr %= 24;
	}
}

/// Initialization of decryption function 5 with a table
///
/// @param key   Key
/// @param seed  Seed
///
const BYTE* CCpz::InitDecryptWithTable5(DWORD key, DWORD seed)
{
	static BYTE decryption_table[256];

	for (size_t i = 0; i < 256; i++)
	{
		decryption_table[i] = static_cast<BYTE>(i);
	}

	for (size_t i = 0; i < 256; i++)
	{
		std::swap(decryption_table[(key >> 16) & 0xFF], decryption_table[key & 0xFF]);
		std::swap(decryption_table[(key >> 8) & 0xFF], decryption_table[(key >> 24) & 0xFF]);

		key = seed + _lrotr(key, 2) * 0x1A743125;
	}

	return decryption_table;
}

/// Decoding function 5 with a table
///
/// @param target           Decoded data
/// @param size             Decoding size
/// @param decryption_table Decryption table
/// @param key              Key
///
void CCpz::DecryptWithTable5(BYTE* target, DWORD size, const BYTE* decryption_table, DWORD key)
{
	for (size_t i = 0; i < size; i++)
	{
		target[i] = decryption_table[target[i] ^ key];
	}
}

/// Decoding CPZ5 data
///
/// @param target           Decoded data
/// @param size             Decoding size
/// @param decryption_table Decryption table
/// @param key              Key
/// @param seed             Seed
///
void CCpz::DecryptOfData5(BYTE* target, DWORD size, const BYTE* decryption_table, const DWORD* key, DWORD seed)
{
	static const DWORD crypt[] =
	{
		0xCD90F089, 0xE982B782, 0xA282AB88, 0xCD82718E, 0x52838A83, 0xA882AA82, 0x7592648E, 0xB582AB82,
		0xE182BF82, 0xDC82A282, 0x4281B782, 0xED82F48E, 0xBF82EA82, 0xA282E182, 0xB782DC82, 0x6081E682,
		0xC6824181, 0xA482A282, 0xE082A982, 0xF48EA482, 0xBF82C182, 0xA282E182, 0xB582DC82, 0xF481BD82
	};

	static const BYTE* byte_crypt = (const BYTE*)crypt;

	DWORD table[24];
	BYTE* byte_table = (BYTE*)table;
	DWORD key_value = key[1] >> 2;

	for (size_t i = 0; i < 96; i++)
	{
		byte_table[i] = decryption_table[byte_crypt[i]] ^ (BYTE)key_value;
	}

	for (size_t i = 0; i < 24; i++)
	{
		table[i] ^= seed;
	}

	DWORD table_ptr = 9;
	key_value = 0x2547A39E;

	for (DWORD i = 0; i < (size >> 2); i++)
	{
		*(DWORD*)target = key[key_value & 3] ^ ((*(DWORD*)target ^ table[(key_value >> 6) & 0x0F] ^ (table[table_ptr++] >> 1)) - seed);

		table_ptr &= 0x0F;
		key_value += *(DWORD*)target + seed;

		target += 4;
	}

	DecryptWithTable5(target, size & 3, decryption_table, 0xBC);
}

/// Callback function 3 from PB3B
///
/// @param target      Data
/// @param target_size Size
/// @param archive     Archive
/// @param file_info   File info
///
void CCpz::OnDecrypt3FromPB3B(u8* target, size_t target_size, CArcFile* archive, const SFileInfo& file_info)
{
	Decrypt3(target, target_size, file_info.key);
}

/// Callback function 5 from PB3B
///
///
/// @param target       Data
/// @param target_size  Size
/// @param archive      Archive
/// @param file_info    File info
///
void CCpz::OnDecrypt5FromPB3B(u8* target, size_t target_size, CArcFile* archive, const SFileInfo& file_info)
{
	const auto* cpz5_header = reinterpret_cast<const SCPZ5Header*>(archive->GetHeader());
	const u8* table = InitDecryptWithTable5(cpz5_header->adwMD5[3], cpz5_header->dwIndexKey);

	DecryptOfData5(target, target_size, table, cpz5_header->adwMD5, file_info.key);
}
