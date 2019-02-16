#include "StdAfx.h"
#include "Extract/TCD/TCD3.h"

#include "ArcFile.h"

namespace
{
struct STCD3IndexInfo
{
	u32 file_size;             // File size
	u32 index_offset;          // Absolute offset value to the index
	u32 dir_count;             // Number of folders
	u32 dir_name_length;       // Folder name length (4 byte unites. Example F SYSTEM == 8 bytes)
	u32 file_count;            // Number of files
	u32 file_name_length;      // File name length
	u32 file_count_ex;         // File count (Extended version)
	u32 file_name_length_ex;   // File name length (Extended version)
};

struct STCD3DirInfo
{
	u32 file_count;            // Number of files
	u32 file_name_offset;      // File name offset value
	u32 file_offset;           // File offset value
	u32 reserved;              // Reserved
};
} // Anonymous namespace

/// Mount
///
/// @param archive Archive
///
bool CTCD3::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".TCD"))
		return false;

	if (memcmp(archive->GetHeader(), "TCD3", 4) != 0)
		return false;

	archive->SeekHed(4);

	// Get file count
	u32 files;
	archive->ReadU32(&files);

	// Get index info
	std::array<STCD3IndexInfo, 5> tcd3_index_info;
	archive->Read(tcd3_index_info.data(), sizeof(STCD3IndexInfo) * tcd3_index_info.size());

	// Create Key Table
	static constexpr std::array<u8, 5> key{{
		0xB7, 0x39, 0x24, 0x8D, 0x8D
	}};

	// Create extension table
	static const std::array<YCString, 5> file_exts{{
		_T(".tct"), _T(".tsf"), _T(".spd"), _T(".ogg"), _T(".wav")
	}};

	// Read index
	for (size_t file_type = 0; file_type < tcd3_index_info.size(); file_type++)
	{
		if (tcd3_index_info[file_type].file_size == 0)
		{
			// Index does not exist
			continue;
		}

		// Go to index
		archive->SeekHed(tcd3_index_info[file_type].index_offset);

		// Folder name
		const u32 all_dir_name_length = tcd3_index_info[file_type].dir_name_length * tcd3_index_info[file_type].dir_count;
		std::vector<u8> all_dir_names(all_dir_name_length);
		archive->Read(all_dir_names.data(), all_dir_names.size());

		// Decode folder name
		for (size_t i = 0; i < all_dir_names.size(); i++)
		{
			all_dir_names[i] -= key[file_type];
		}

		// Get folder info
		std::vector<STCD3DirInfo> tcd3_dir_info(tcd3_index_info[file_type].dir_count);
		archive->Read(tcd3_dir_info.data(), sizeof(STCD3DirInfo) * tcd3_dir_info.size());

		// File name
		const u32 all_file_name_length = tcd3_index_info[file_type].file_name_length * tcd3_index_info[file_type].file_count;
		std::vector<u8> all_file_names(all_file_name_length);
		archive->Read(all_file_names.data(), all_file_names.size());

		// Decode file name
		for (size_t i = 0; i < all_file_names.size(); i++)
		{
			all_file_names[i] -= key[file_type];
		}

		// File offset
		const u32 all_file_offset_length = tcd3_index_info[file_type].file_count + 1;
		std::vector<u32> all_file_offsets(all_file_offset_length);
		archive->Read(all_file_offsets.data(), sizeof(u32)* all_file_offsets.size());

		// Store Info
		for (size_t dir = 0; dir < tcd3_index_info[file_type].dir_count; dir++)
		{
			// Get folder info
			TCHAR dir_name[_MAX_DIR];
			memcpy(dir_name, &all_dir_names[tcd3_index_info[file_type].dir_name_length * dir], tcd3_index_info[file_type].dir_name_length);

			for (size_t file = 0; file < tcd3_dir_info[dir].file_count; file++)
			{
				// Get file name
				TCHAR file_name[_MAX_FNAME];
				memcpy(file_name, &all_file_names[tcd3_dir_info[dir].file_name_offset + tcd3_index_info[file_type].file_name_length * file], tcd3_index_info[file_type].file_name_length);
				file_name[tcd3_index_info[file_type].file_name_length] = _T('\0');

				// Folder name + File name + Extension
				TCHAR full_name[_MAX_PATH];
				_stprintf(full_name, _T("%s\\%s%s"), dir_name, file_name, file_exts[file_type].GetString());

				// Store Info
				SFileInfo file_info;
				file_info.name = full_name;
				file_info.start = all_file_offsets[tcd3_dir_info[dir].file_offset + file + 0];
				file_info.end = all_file_offsets[tcd3_dir_info[dir].file_offset + file + 1];
				file_info.sizeCmp = file_info.end - file_info.start;
				file_info.sizeOrg = file_info.sizeCmp;

				archive->AddFileInfo(file_info);
			}
		}
	}

	return true;
}

/// RLE Decompression (Type 2)
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Input data
/// @param src_size Input data size
///
bool CTCD3::DecompRLE2(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	const u32 offset = *reinterpret_cast<const u32*>(&src[0]);
	const u32 pixel_count = *reinterpret_cast<const u32*>(&src[4]);

	size_t src_header_ptr = 8;
	size_t src_data_ptr = offset;
	size_t dst_ptr = 0;

	while (src_header_ptr < offset && src_data_ptr < src_size && dst_ptr < dst_size)
	{
		const u16 work = *reinterpret_cast<const u16*>(&src[src_header_ptr]);

		src_header_ptr += 2;

		const u16 type = work >> 14;
		const u16 length = work & 0x3FFF;

		if ((dst_ptr + (length * 4)) > dst_size)
		{
			MessageBox(nullptr, _T("The output buffer to decompress RLE2 is too small."), _T("Error"), 0);
		}

		switch (type)
		{
		case 0: // Fill in 0
			for (u32 i = 0; i < length; i++)
			{
				for (u32 j = 0; j < 4; j++)
				{
					dst[dst_ptr++] = 0x00;
				}
			}
			break;

		case 1:  // Alpha value 0xFF
			if ((src_data_ptr + (length * 3)) > src_size)
			{
				MessageBox(nullptr, _T("Input buffer to decompress RLE2 is too small."), _T("Error"), 0);
			}

			for (u32 i = 0; i < length; i++)
			{
				for (u32 j = 0; j < 3; j++)
				{
					dst[dst_ptr++] = src[src_data_ptr++];
				}

				dst[dst_ptr++] = 0xFF;
			}
			break;

		default: // Alpha values obtained from header
			if ((src_data_ptr + (length * 3)) > src_size)
			{
				MessageBox(nullptr, _T("Input buffer needed to decompress RLE2 is too small."), _T("Error"), 0);
			}

			if ((src_header_ptr + length) > offset)
			{
				MessageBox(nullptr, _T("Input buffer needed to decompress RLE2 is too small."), _T("Error"), 0);
			}

			for (u32 i = 0; i < length; i++)
			{
				for (u32 j = 0; j < 3; j++)
				{
					dst[dst_ptr++] = src[src_data_ptr++];
				}

				dst[dst_ptr++] = src[src_header_ptr++];
			}
			break;
		}
	}

	return true;
}
