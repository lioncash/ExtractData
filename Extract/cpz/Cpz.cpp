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
	u8 header[16];
	archive->Read(header, sizeof(header));
	const u32 files = *reinterpret_cast<u32*>(&header[4]);
	const u32 index_size = *reinterpret_cast<u32*>(&header[8]);

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Decrypt the index
	Decrypt1(index.data(), index.size());
	const size_t offset = index.size() + 16;

	size_t index_ptr = 0;
	for (size_t i = 0; i < files; i++)
	{
		TCHAR file_name[256];
		lstrcpy(file_name, reinterpret_cast<LPCTSTR>(&index[index_ptr + 24]));

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.sizeCmp = *reinterpret_cast<u32*>(&index[index_ptr + 4]);
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.start = *reinterpret_cast<u32*>(&index[index_ptr + 8]) + offset;
		file_info.end = file_info.start + file_info.sizeCmp;
		archive->AddFileInfo(file_info);

		index_ptr += *reinterpret_cast<u32*>(&index[index_ptr]);
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
	u8 header[20];
	archive->Read(header, sizeof(header));

	const u32 files = *reinterpret_cast<u32*>(&header[4]) ^ 0xE47C59F3;
	const u32 index_size = *reinterpret_cast<u32*>(&header[8]) ^ 0x3F71DE2A;
	const u32 key = *reinterpret_cast<u32*>(&header[16]) ^ 0x77777777 ^ 0x37A9F45B;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Decrypt index
	Decrypt2(index.data(), index.size(), key);
	const size_t offset = index.size() + 20;

	size_t index_ptr = 0;
	for (size_t i = 0; i < files; i++)
	{
		TCHAR file_name[256];
		lstrcpy(file_name, reinterpret_cast<LPCTSTR>(&index[index_ptr + 24]));

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.sizeCmp = *reinterpret_cast<u32*>(&index[index_ptr + 4]);
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.start = *reinterpret_cast<u32*>(&index[index_ptr + 8]) + offset;
		file_info.end = file_info.start + file_info.sizeCmp;
		file_info.key = *reinterpret_cast<u32*>(&index[index_ptr + 20]) ^ 0x796C3AFD;
		archive->AddFileInfo(file_info);

		index_ptr += *reinterpret_cast<u32*>(&index[index_ptr]);
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
	u8 header[20];
	archive->Read(header, sizeof(header));

	const u32 files = *reinterpret_cast<u32*>(&header[4]) ^ 0x5E9C4F37;
	const u32 index_size = *reinterpret_cast<u32*>(&header[8]) ^ 0xF32AED17;
	const u32 key = *reinterpret_cast<u32*>(&header[16]) ^ 0xDDDDDDDD ^ 0x7BF4A539;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Decrypt index
	Decrypt3(index.data(), index.size(), key);
	const size_t offset = index.size() + 20;

	size_t index_ptr = 0;
	for (size_t i = 0; i < files; i++)
	{
		TCHAR file_name[_MAX_FNAME];
		lstrcpy(file_name, reinterpret_cast<LPCTSTR>(&index[index_ptr + 24]));

		// ファイル数が多いので、フォルダ名を付けてフォルダ分割できるように変更
		// プリミティブリンク用の処理なので、他タイトルで不具合出るかも
		YCString dir_name;
		LPTSTR dir_name_end_pos = _tcschr(file_name, _T('-'));

		if (dir_name_end_pos != nullptr)
		{
			dir_name.Append(file_name, (dir_name_end_pos + 3 - file_name));
			dir_name += _T("\\");
		}

		// Add to listview
		SFileInfo file_info;
		file_info.name = dir_name + file_name;
		file_info.sizeCmp = *reinterpret_cast<u32*>(&index[index_ptr + 4]);
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.start = *reinterpret_cast<u32*>(&index[index_ptr + 8]) + offset;
		file_info.end = file_info.start + file_info.sizeCmp;
		file_info.key = *reinterpret_cast<u32*>(&index[index_ptr + 20]) ^ 0xC7F5DA63;
		archive->AddFileInfo(file_info);

		index_ptr += *reinterpret_cast<u32*>(&index[index_ptr]);
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
	header->num_dirs ^= 0xFE3A53D9;
	header->total_dir_index_size ^= 0x37F298E7;
	header->total_file_index_size ^= 0x7A6F3A2C;
	header->md5[0] ^= 0x43DE7C19;
	header->md5[1] ^= 0xCC65F415;
	header->md5[2] ^= 0xD016A93C;
	header->md5[3] ^= 0x97A3BA9A;
	header->index_key ^= 0xAE7D39BF;
	header->unknown2[0] ^= 0xFB73A955;
	header->unknown2[1] ^= 0x37ACF831;

	// Prepare MD5 data
	u32 md5_data[16];
	md5_data[0] = header->md5[0];
	md5_data[1] = header->md5[1];
	md5_data[2] = header->md5[2];
	md5_data[3] = header->md5[3];

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
	header->md5[0] = md5_result.adwABCD[3];
	header->md5[1] = md5_result.adwABCD[1];
	header->md5[2] = md5_result.adwABCD[2];
	header->md5[3] = md5_result.adwABCD[0];

	// Read index
	const u32 index_size = header->total_dir_index_size + header->total_file_index_size;
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Decode entire directory index
	Decrypt5(index.data(), index.size(), header->index_key ^ 0x3795B39A);
	const u8* decryption_table = InitDecryptWithTable5(header->index_key, header->md5[1]);
	DecryptWithTable5(index.data(), header->total_dir_index_size, decryption_table, 0x3A);

	// Set key
	u32 key[4];
	key[0] = header->md5[0] ^ (header->index_key + 0x76A3BF29);
	key[1] = header->md5[1] ^ header->index_key;
	key[2] = header->md5[2] ^ (header->index_key + 0x10000000);
	key[3] = header->md5[3] ^ header->index_key;

	// Decrypt the total directory index
	size_t key_ptr = 0;
	u32 seed = 0x76548AEF;
	u8* work_ptr = index.data();

	for (size_t i = 0; i < header->total_dir_index_size >> 2; i++)
	{
		const u32 work = (*reinterpret_cast<u32*>(work_ptr) ^ key[key_ptr++]) - 0x4A91C262;

		*reinterpret_cast<u32*>(work_ptr) = _lrotl(work, 3) - seed;

		key_ptr &= 3;
		work_ptr += 4;
		seed += 0x10FB562A;
	}

	for (size_t i = 0; i < (header->total_dir_index_size & 3); i++)
	{
		*work_ptr++ = ((key[key_ptr++] >> 6) ^ *work_ptr) + 0x37;

		key_ptr &= 3;
	}

	// Initialize decryption table
	decryption_table = InitDecryptWithTable5(header->index_key, header->md5[2]);

	// Get file information
	const u8* current_dir_index_ptr = index.data();

	for (u32 i = 0; i < header->num_dirs; i++)
	{
		const u8* next_dir_index_ptr = current_dir_index_ptr + *reinterpret_cast<const u32*>(&current_dir_index_ptr[0]);
		const u32 current_file_index_offset = *reinterpret_cast<const u32*>(&current_dir_index_ptr[8]);
		u32 next_file_index_offset;

		if ((i + 1) >= header->num_dirs)
		{
			// The last directory

			next_file_index_offset = header->total_file_index_size;
		}
		else
		{
			next_file_index_offset = *reinterpret_cast<const u32*>(&next_dir_index_ptr[8]);
		}

		// Decrypt the current file index
		const u32 current_file_index_size = next_file_index_offset - current_file_index_offset;
		u8* current_file_index = &index[header->total_dir_index_size + current_file_index_offset];
		DecryptWithTable5(current_file_index, current_file_index_size, decryption_table, 0x7E);

		// Set key
		const u32 entry_key = *reinterpret_cast<const u32*>(&current_dir_index_ptr[12]);
		key[0] = header->md5[0] ^ entry_key;
		key[1] = header->md5[1] ^ (entry_key + 0x112233);
		key[2] = header->md5[2] ^ entry_key;
		key[3] = header->md5[3] ^ (entry_key + 0x34258765);

		// Decrypt current file index
		seed = 0x2A65CB4E;
		work_ptr = current_file_index;
		key_ptr = 0;

		for (u32 j = 0; j < current_file_index_size >> 2; j++)
		{
			*reinterpret_cast<u32*>(work_ptr) = _lrotl((*reinterpret_cast<u32*>(work_ptr) ^ key[key_ptr++]) - seed, 2) + 0x37A19E8B;

			key_ptr &= 3;
			work_ptr += 4;
			seed -= 0x139FA9B;
		}

		for (u32 j = 0; j < (current_file_index_size & 3); j++)
		{
			*work_ptr++ = (*work_ptr ^ static_cast<u8>(key[key_ptr++] >> 4)) + 5;

			key_ptr &= 3;
		}

		// Get file information
		const u8* file_entry = current_file_index;

		for (u32 j = 0; j < *reinterpret_cast<const u32*>(&current_dir_index_ptr[4]); j++)
		{
			// Get file name
			TCHAR file_name[_MAX_FNAME];
			if (strcmp(reinterpret_cast<const char*>(&current_dir_index_ptr[16]), "root") == 0)
			{
				_stprintf(file_name, _T("%s"),
				          reinterpret_cast<const char*>(&file_entry[24]));
			}
			else
			{
				_stprintf(file_name, _T("%s\\%s"),
				          reinterpret_cast<const char*>(&current_dir_index_ptr[16]),
				          reinterpret_cast<const char*>(&file_entry[24]));
			}

			// Additional file information
			SFileInfo file_info;
			file_info.name = file_name;
			file_info.start = *reinterpret_cast<const u64*>(&file_entry[4]) + sizeof(SCPZ5Header) + index_size;
			file_info.sizeCmp = *reinterpret_cast<const u32*>(&file_entry[12]);
			file_info.sizeOrg = file_info.sizeCmp;
			file_info.end = file_info.start + file_info.sizeCmp;
			file_info.key = (header->index_key ^ (*reinterpret_cast<const u32*>(&current_dir_index_ptr[12]) + *reinterpret_cast<const u32*>(&file_entry[20]))) + header->num_dirs + 0xA3D61785;
			archive->AddFileInfo(file_info);

			// Go to the next file entry
			file_entry += *reinterpret_cast<const u32*>(&file_entry[0]);
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
	std::vector<u8> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	// Decryption
	Decrypt1(src.data(), src.size());

	// Output
	if (file_info->format == _T("PB2"))
	{
		CPB2A pb2a;
		pb2a.Decode(archive, src.data(), src.size());
	}
	else if (file_info->format == _T("MSK"))
	{
		const s32 width = *reinterpret_cast<s32*>(&src[8]);
		const s32 height = *reinterpret_cast<s32*>(&src[12]);
		const u32 dst_size = static_cast<u32>(width * height);

		CImage image;
		image.Init(archive, width, height, 8);
		image.WriteReverse(&src[16], dst_size);
	}
	else
	{
		archive->OpenFile();
		archive->WriteFile(src.data(), src.size());
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
	std::vector<u8> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	// Decryption
	Decrypt2(src.data(), src.size(), file_info->key);

	// Output
	if (file_info->format == _T("PB2"))
	{
		CPB2A pb2a;
		pb2a.Decode(archive, src.data(), src.size());
	}
	else if (file_info->format == _T("MSK"))
	{
		const s32 width = *reinterpret_cast<s32*>(&src[8]);
		const s32 height = *reinterpret_cast<s32*>(&src[12]);
		const u32 dst_size = static_cast<u32>(width * height);

		CImage image;
		image.Init(archive, width, height, 8);
		image.WriteReverse(&src[16], dst_size);
	}
	else
	{
		archive->OpenFile();
		archive->WriteFile(src.data(), src.size());
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
	std::vector<u8> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	// Decryption
	Decrypt3(src.data(), src.size(), file_info->key);

	if (file_info->format == _T("PB3"))
	{
		CPB3B pb3b;
		pb3b.Decode(archive, src.data(), src.size(), OnDecrypt3FromPB3B);
	}
	else
	{
		archive->OpenFile();
		archive->WriteFile(src.data(), src.size());
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
	std::vector<u8> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	// Decryption
	const u8* table = InitDecryptWithTable5(cpz5_header->md5[3], cpz5_header->index_key);
	DecryptOfData5(src.data(), src.size(), table, cpz5_header->md5, file_info->key);

	if (file_info->format == _T("PB3"))
	{
		CPB3B pb3b;
		pb3b.Decode(archive, src.data(), src.size(), OnDecrypt5FromPB3B);
	}
	else
	{
		archive->OpenFile();
		archive->WriteFile(src.data(), src.size());
	}

	return true;
}

/// Decryption function 1
///
/// @param target Decoded data
/// @param size   Decoding size
///
void CCpz::Decrypt1(u8* target, size_t size)
{
	static constexpr u8 crypt[] = "掴乱跨虎規日壅諺庫誤絞豪股誇砧後口糊己交\x8B\xFE謙倖規弭胡綿戸湖候誇騎";

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
void CCpz::Decrypt2(u8* target, size_t size, u32 key)
{
	static constexpr std::array<u32, 16> crypt{{
		0x3A68CDBF, 0xD3C3A711, 0x8414876E, 0x657BEFDB,
		0xCDD7C125, 0x09328580, 0x288FFEDD, 0x99EBF13A,
		0x5A471F95, 0x1EA3F4F1, 0xF4FF524E, 0xD358E8A9,
		0xC5B71015, 0xA913046F, 0x2D6FD2BD, 0x68C8BE19
	}};

	u32 shift = key ^ 0x50000000;
	for (u32 i = 0; i < 7; i++)
	{
		shift = (shift >> 4) ^ key;
	}
	shift = (shift & 0x0F) + 8;

	size_t table_ptr = 0;
	for (size_t i = 0; i < size >> 2; i++)
	{
		const u32 work = ((crypt[table_ptr++] + key) ^ *reinterpret_cast<u32*>(target)) - 0x15C3E7;

		*reinterpret_cast<u32*>(target) = (work << (32 - shift)) | (work >> shift);

		table_ptr &= 0x0F;
		target += 4;
	}

	for (size_t i = 0, j = 0; i < (size & 3); i++, j += 4)
	{
		*target++ = static_cast<u8>((((crypt[table_ptr++] + key) >> j) ^ *target) + 0x37);

		table_ptr &= 0x0F;
	}
}

/// Decryption function 3
///
/// @param target Decoded data
/// @param size   Decoding size
/// @param key    Key
///
void CCpz::Decrypt3(u8* target, size_t size, u32 key)
{
	static constexpr std::array<u32, 16> crypt{{
		0x4D0D4A5E, 0xB3ABF3E1, 0x3C37336D, 0x86C3F5F3,
		0x7D4F9B89, 0x58D7DE11, 0x6367778D, 0xA5F34629,
		0x067FA4B5, 0xED0AE742, 0xB19450CC, 0xE7204A5A,
		0xD9AF04F5, 0x5D3B687F, 0xC1C7A6FD, 0xFC502289
	}};

	std::array<u32, 16> table;
	for (size_t i = 0; i < table.size(); i++)
	{
		table[i] = crypt[i] + key;
	}

	u32 shift = key;
	for (u32 i = 0; i < 7; i++)
	{
		shift = (shift >> 4) ^ key;
	}

	shift = ((shift ^ 0xFFFFFFFD) & 0x0F) + 8;

	size_t table_ptr = 3;
	for (size_t i = 0; i < size >> 2; i++)
	{
		const u32 work = (table[table_ptr++] ^ *reinterpret_cast<u32*>(target)) + 0x6E58A5C2;

		*reinterpret_cast<u32*>(target) = (work << shift) | (work >> (32 - shift));

		table_ptr &= 0x0F;
		target += 4;
	}

	for (size_t i = size & 3; i > 0; i--)
	{
		*target++ = static_cast<u8>(((table[table_ptr++] >> (i * 4)) ^ *target) + 0x52);

		table_ptr &= 0x0F;
	}
}

/// Decryption function 5
///
/// @param target Decoded data
/// @param size   Decoding size
/// @param key    Key
///
void CCpz::Decrypt5(u8* target, size_t size, u32 key)
{
	static constexpr std::array<u32, 24> crypt{{
		0xCD90F089, 0xE982B782, 0xA282AB88, 0xCD82718E, 0x52838A83, 0xA882AA82, 0x7592648E, 0xB582AB82,
		0xE182BF82, 0xDC82A282, 0x4281B782, 0xED82F48E, 0xBF82EA82, 0xA282E182, 0xB782DC82, 0x6081E682,
		0xC6824181, 0xA482A282, 0xE082A982, 0xF48EA482, 0xBF82C182, 0xA282E182, 0xB582DC82, 0xF481BD82
	}};

	std::array<u32, 24> table;
	for (size_t i = 0; i < table.size(); i++)
	{
		table[i] = crypt[i] - key;
	}

	u32 shift = key;
	for (u32 i = 0; i < 3; i++)
	{
		shift = (shift >> 8) ^ key;
	}
	shift = ((shift ^ 0xFFFFFFFB) & 0x0F) + 7;

	size_t table_ptr = 5;
	for (size_t i = 0; i < size >> 2; i++)
	{
		const u32 work = (table[table_ptr++] ^ *reinterpret_cast<u32*>(target)) + 0x784C5962;

		*reinterpret_cast<u32*>(target) = _lrotr(work, shift) + 0x01010101;

		table_ptr %= 24;
		target += 4;
	}

	for (size_t i = size & 3; i > 0; i--)
	{
		*target++ = static_cast<u8>(((table[table_ptr++] >> (i * 4)) ^ *target) - 0x79);

		table_ptr %= 24;
	}
}

/// Initialization of decryption function 5 with a table
///
/// @param key   Key
/// @param seed  Seed
///
const u8* CCpz::InitDecryptWithTable5(u32 key, u32 seed)
{
	static std::array<u8, 256> decryption_table;

	for (size_t i = 0; i < 256; i++)
	{
		decryption_table[i] = static_cast<u8>(i);
	}

	for (size_t i = 0; i < 256; i++)
	{
		std::swap(decryption_table[(key >> 16) & 0xFF], decryption_table[key & 0xFF]);
		std::swap(decryption_table[(key >> 8) & 0xFF], decryption_table[(key >> 24) & 0xFF]);

		key = seed + _lrotr(key, 2) * 0x1A743125;
	}

	return decryption_table.data();
}

/// Decoding function 5 with a table
///
/// @param target           Decoded data
/// @param size             Decoding size
/// @param decryption_table Decryption table
/// @param key              Key
///
void CCpz::DecryptWithTable5(u8* target, size_t size, const u8* decryption_table, u32 key)
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
void CCpz::DecryptOfData5(u8* target, size_t size, const u8* decryption_table, const u32* key, u32 seed)
{
	static constexpr std::array<u32, 24> crypt{{
		0xCD90F089, 0xE982B782, 0xA282AB88, 0xCD82718E, 0x52838A83, 0xA882AA82, 0x7592648E, 0xB582AB82,
		0xE182BF82, 0xDC82A282, 0x4281B782, 0xED82F48E, 0xBF82EA82, 0xA282E182, 0xB782DC82, 0x6081E682,
		0xC6824181, 0xA482A282, 0xE082A982, 0xF48EA482, 0xBF82C182, 0xA282E182, 0xB582DC82, 0xF481BD82
	}};

	static const u8* byte_crypt = reinterpret_cast<const u8*>(crypt.data());

	std::array<u32, 24> table;
	u8* byte_table = reinterpret_cast<u8*>(table.data());
	u32 key_value = key[1] >> 2;

	for (size_t i = 0; i < 96; i++)
	{
		byte_table[i] = decryption_table[byte_crypt[i]] ^ static_cast<u8>(key_value);
	}

	for (u32& entry : table)
	{
		entry ^= seed;
	}

	size_t table_ptr = 9;
	key_value = 0x2547A39E;

	for (size_t i = 0; i < size >> 2; i++)
	{
		*reinterpret_cast<u32*>(target) = key[key_value & 3] ^ ((*reinterpret_cast<u32*>(target) ^ table[(key_value >> 6) & 0x0F] ^ (table[table_ptr++] >> 1)) - seed);

		table_ptr &= 0x0F;
		key_value += *reinterpret_cast<u32*>(target) + seed;

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
	const u8* table = InitDecryptWithTable5(cpz5_header->md5[3], cpz5_header->index_key);

	DecryptOfData5(target, target_size, table, cpz5_header->md5, file_info.key);
}
