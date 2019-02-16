#include "StdAfx.h"
#include "Extract/TCD/TCD2.h"

#include "ArcFile.h"
#include "Common.h"

namespace
{
struct STCD2IndexInfo
{
	u32 file_size;             // File size
	u32 file_count;            // Number of files
	u32 dir_count;             // Number of folders
	u32 index_offset;          // Absolute offset value to the index
	u32 dir_name_length;       // Folder name length
	u32 file_name_length;      // File name length
	u32 file_count_ex;         // File count(Extended version)
	u32 file_name_length_ex;   // File name length(Extended version)
};

struct STCD2DirInfo
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
bool CTCD2::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".TCD"))
		return false;

	if (memcmp(archive->GetHeader(), "TCD2", 4) != 0)
		return false;

	archive->SeekHed(4);

	// Get file count
	u32 files;
	archive->ReadU32(&files);

	// Get index info
	std::array<STCD2IndexInfo, 5> tcd2_index_info;
	archive->Read(tcd2_index_info.data(), sizeof(STCD2IndexInfo) * tcd2_index_info.size());

	// Create key table
	static constexpr std::array<u8, 5> key{{
		0x1F, 0x61, 0x43, 0x76, 0x76
	}};

	// Create extension table
	static const std::array<YCString, 5> file_exts{{
		_T(".tct"), _T(".tsf"), _T(".spd"), _T(".ogg"), _T(".wav")
	}};

	// Read index
	for (size_t file_type = 0; file_type < tcd2_index_info.size(); file_type++)
	{
		if (tcd2_index_info[file_type].file_size == 0)
		{
			// Index doesn't exist
			continue;
		}

		// Go to index
		archive->SeekHed(tcd2_index_info[file_type].index_offset);

		// Folder name
		const u32 all_dir_name_length = tcd2_index_info[file_type].dir_name_length;
		std::vector<u8> all_dir_names(all_dir_name_length);
		archive->Read(all_dir_names.data(), all_dir_names.size());

		// Decode folder name
		for (size_t i = 0; i < all_dir_name_length; i++)
		{
			all_dir_names[i] -= key[file_type];
		}

		// Get folder info
		std::vector<STCD2DirInfo> tcd2_dir_info(tcd2_index_info[file_type].dir_count);
		archive->Read(tcd2_dir_info.data(), sizeof(STCD2DirInfo) * tcd2_dir_info.size());

		// File name
		const u32 all_file_name_length = tcd2_index_info[file_type].file_name_length;
		std::vector<u8> all_file_names(all_file_name_length);
		archive->Read(all_file_names.data(), all_file_names.size());

		// Decode file name
		for (size_t i = 0; i < all_file_name_length; i++)
		{
			all_file_names[i] -= key[file_type];
		}

		// File offset
		const u32 all_file_offset_length = tcd2_index_info[file_type].file_count + 1;
		std::vector<u32> all_file_offsets(all_file_offset_length);
		archive->Read(all_file_offsets.data(), sizeof(u32) * all_file_offsets.size());

		// Store info
		size_t dir_name_ptr = 0;
		for (size_t dir = 0; dir < tcd2_index_info[file_type].dir_count; dir++)
		{
			// Get folder name
			char dir_name[_MAX_DIR];
			strcpy(dir_name, reinterpret_cast<char*>(&all_dir_names[dir_name_ptr]));
			dir_name_ptr += strlen(dir_name) + 1;

			size_t file_name_ptr = 0;
			for (size_t file = 0; file < tcd2_dir_info[dir].file_count; file++)
			{
				// Get file name
				char file_name[_MAX_FNAME];
				strcpy(file_name, reinterpret_cast<char*>(&all_file_names[tcd2_dir_info[dir].file_name_offset + file_name_ptr]));
				file_name_ptr += strlen(file_name) + 1;

				// Folder name + File name + Extension
				TCHAR full_name[_MAX_PATH];
				_stprintf(full_name, _T("%s\\%s%s"), dir_name, file_name, file_exts[file_type].GetString());

				// Store info
				SFileInfo file_info;
				file_info.name = full_name;
				file_info.start = all_file_offsets[tcd2_dir_info[dir].file_offset + file + 0];
				file_info.end = all_file_offsets[tcd2_dir_info[dir].file_offset + file + 1];
				file_info.size_cmp = file_info.end - file_info.start;
				file_info.size_org = file_info.size_cmp;

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
bool CTCD2::DecompRLE2(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	const u32 offset = *reinterpret_cast<const u32*>(&src[0]);
	const u32 pixel_count = *reinterpret_cast<const u32*>(&src[4]);

	size_t src_header_ptr = 8;
	size_t src_data_ptr = offset;
	size_t dst_ptr = 0;

	while (src_header_ptr < offset && src_data_ptr < src_size && dst_ptr < dst_size)
	{
		const u8 work = src[src_header_ptr++];

		switch (work)
		{
		case 0: // Fill in 0
		{
			const u16 length = src[src_header_ptr++] + 1;
			for (u32 i = 0; i < length; i++)
			{
				for (u32 j = 0; j < 4; j++)
				{
					dst[dst_ptr++] = 0x00;
				}
			}
			break;
		}

		case 1: // Alpha value 0xFF
		{
			const u16 length = src[src_header_ptr++] + 1;
			for (u32 i = 0; i < length; i++)
			{
				for (u32 j = 0; j < 3; j++)
				{
					dst[dst_ptr++] = src[src_data_ptr++];
				}

				dst[dst_ptr++] = 0xFF;
			}
			break;
		}

		default: // Alpha values 0x01~0xFE
			for (u32 j = 0; j < 3; j++)
			{
				dst[dst_ptr++] = src[src_data_ptr++];
			}
			dst[dst_ptr++] = ~(work - 1);
			break;
		}
	}

	return true;
}
