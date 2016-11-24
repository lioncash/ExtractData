#include "StdAfx.h"
#include "Yuris.h"

bool CYuris::Mount(CArcFile* archive)
{
	if (MountYPF(archive))
		return true;

	if (MountYMV(archive))
		return true;

	return false;
}

bool CYuris::MountYPF(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".ypf"))
		return false;

	if (memcmp(archive->GetHed(), "YPF", 3) != 0)
		return false;

	static constexpr std::array<u8, 256> file_name_length_table =
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x0B, 0x0A, 0x09, 0x10, 0x13, 0x0E, 0x0F,
		0x0C, 0x19, 0x12, 0x0D, 0x14, 0x1B, 0x16, 0x17, 0x18, 0x11, 0x1A, 0x15, 0x1E, 0x1D, 0x1C, 0x1F,
		0x23, 0x21, 0x22, 0x20, 0x24, 0x25, 0x29, 0x27, 0x28, 0x26, 0x2A, 0x2B, 0x2F, 0x2D, 0x32, 0x2C,
		0x30, 0x31, 0x2E, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
		0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
		0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
		0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
		0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
		0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
		0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
		0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
		0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
		0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
		0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
	};

	static constexpr std::array<u8, 43> not_use_word =
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
		0x20, 0x22, 0x2A, 0x2C, 0x3A, 0x3B, 0x3C, 0x3E, 0x3F, 0x7C, 0x7F
	};

	// Get file count
	u32 num_files;
	archive->Seek(8, FILE_BEGIN);
	archive->ReadU32(&num_files);

	// Get index size
	u32 index_size;
	archive->ReadU32(&index_size);

	// Get index
	std::vector<u8> index(index_size);
	archive->Seek(16, FILE_CURRENT);
	archive->Read(index.data(), index.size());
	const u8* index_ptr = index.data();

	// Decoding test
	static constexpr std::array<u32, 2> file_info_lengths = { 26, 18 };

	bool success = false;
	u32 file_info_length = 0;
	u8  key = 0;

	for (size_t i = 0; i < file_info_lengths.size(); i++)
	{
		u32 count = 0;
		u32 index_offset = 0;
		key = 0;

		for (; count < num_files; count++)
		{
			// Decoded value of the filename

			index_offset += 4;

			if (index_offset >= index_size)
			{
				break;
			}

			char file_name[256];
			const u8 length = file_name_length_table[255 - index_ptr[index_offset]];

			index_offset += 1;
			if (index_offset >= index_size)
			{
				break;
			}

			if (index_offset + length >= index_size)
			{
				break;
			}

			for (size_t j = 0; j < length && key == 0; j++)
			{
				file_name[j] = index_ptr[index_offset + j] ^ 0xff;

				if (IsDBCSLeadByte(file_name[j]))
				{
					// Double-byte character
					j++;
					continue;
				}

				for (size_t k = 0; k < not_use_word.size(); k++)
				{
					if (file_name[j] == not_use_word[k])
					{
						key = 0x40;
						break;
					}
				}
			}

			index_offset += length + file_info_lengths[i];
		}

		if (count == num_files && index_offset == index_size)
		{
			// Successful decoding
			success = true;
			file_info_length = file_info_lengths[i];
			break;
		}
	}

	if (!success)
	{
		// Anomaly occurred
		return false;
	}

	for (u32 i = 0; i < num_files; i++)
	{
		// Filename
		TCHAR file_name[256];
		const u8 length = file_name_length_table[255 - index_ptr[4]];

		for (size_t j = 0; j < length; j++)
		{
			file_name[j] = index_ptr[5 + j] ^ 0xFF ^ key;
		}

		file_name[length] = _T('\0');

		const u8 cmp = index_ptr[5 + length + 1];

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.sizeOrg = *(const u32*)&index_ptr[5 + length + 2];
		file_info.sizeCmp = *(const u32*)&index_ptr[5 + length + 6];
		file_info.start = *(const u32*)&index_ptr[5 + length + 10];
		file_info.end = file_info.start + file_info.sizeCmp;
		if (cmp)
			file_info.format = _T("zlib");

		archive->AddFileInfo(file_info);

		index_ptr += 5 + length + file_info_length;
	}

	return true;
}

/// YMV Mounting
bool CYuris::MountYMV(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".ymv"))
		return false;

	if (memcmp(archive->GetHed(), "YSMV", 4) != 0)
		return false;

	archive->SeekHed(8);

	// Get file count
	u32 num_files;
	archive->ReadU32(&num_files);
	archive->SeekCur(4);

	// Get index
	std::vector<u32> offset_indices(num_files);
	std::vector<u32> size_indices(num_files);
	archive->Read(offset_indices.data(), sizeof(u32) * num_files);
	archive->Read(size_indices.data(), sizeof(u32) * num_files);

	// Additional file information
	for (u32 i = 0; i < num_files; i++)
	{
		SFileInfo file_info;
		file_info.name.Format(_T("%s_%06u.jpg"), archive->GetArcName().GetFileTitle(), i);
		file_info.start = offset_indices[i];
		file_info.sizeCmp = size_indices[i];
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.end = file_info.start + file_info.sizeCmp;

		archive->AddFileInfo(file_info);
	}

	return true;
}

/// Decoding
bool CYuris::Decode(CArcFile* archive)
{
	if (DecodeYMV(archive))
		return true;

	return false;
}

/// YMV Decoding
bool CYuris::DecodeYMV(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".ymv"))
		return false;

	if (memcmp(archive->GetHed(), "YSMV", 4) != 0)
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Reading
	std::vector<u8> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	// Decoding
	for (size_t i = 0; i < src.size(); i++)
	{
		const u8 key = (i & 0x0F) + 16;

		src[i] ^= key;
	}

	// Output
	archive->OpenFile();
	archive->WriteFile(src.data(), src.size());
	archive->CloseFile();

	return true;
}
