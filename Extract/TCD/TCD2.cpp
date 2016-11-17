#include "stdafx.h"
#include "../../Image.h"
#include "../../Sound/Ogg.h"
#include "TCD2.h"

/// Mount
///
/// @param archive Archive
///
bool CTCD2::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".TCD"))
		return false;

	if (memcmp(archive->GetHed(), "TCD2", 4) != 0)
		return false;

	archive->SeekHed(4);

	// Get file count
	DWORD files;
	archive->Read(&files, 4);

	// Get index info
	STCD2IndexInfo tcd2_index_info[5];
	archive->Read(tcd2_index_info, (sizeof(STCD2IndexInfo)* 5));

	// Create key table
	static constexpr BYTE key[5] =
	{
		0x1F, 0x61, 0x43, 0x76, 0x76
	};

	// Create extension table
	static const YCString file_exts[5] =
	{
		_T(".tct"), _T(".tsf"), _T(".spd"), _T(".ogg"), _T(".wav")
	};

	// Read index
	for (DWORD file_type = 0; file_type < 5; file_type++)
	{
		if (tcd2_index_info[file_type].file_size == 0)
		{
			// Index doesn't exist
			continue;
		}

		// Go to index
		archive->SeekHed(tcd2_index_info[file_type].index_offset);

		// Folder name
		const DWORD all_dir_name_length = tcd2_index_info[file_type].dir_name_length;
		YCMemory<BYTE> all_dir_names(all_dir_name_length);
		archive->Read(&all_dir_names[0], all_dir_name_length);

		// Decode folder name
		for (size_t i = 0; i < all_dir_name_length; i++)
		{
			all_dir_names[i] -= key[file_type];
		}

		// Get folder info
		YCMemory<STCD2DirInfo> tcd2_dir_info(tcd2_index_info[file_type].dir_count);
		archive->Read(&tcd2_dir_info[0], (sizeof(STCD2DirInfo)* tcd2_index_info[file_type].dir_count));

		// File name
		const DWORD    all_file_name_length = tcd2_index_info[file_type].file_name_length;
		YCMemory<BYTE> all_file_names(all_file_name_length);
		archive->Read(&all_file_names[0], all_file_name_length);

		// Decode file name
		for (size_t i = 0; i < all_file_name_length; i++)
		{
			all_file_names[i] -= key[file_type];
		}

		// File offset
		const DWORD     all_file_offset_length = (tcd2_index_info[file_type].file_count + 1);
		YCMemory<DWORD> all_file_offsets(all_file_offset_length);
		archive->Read(&all_file_offsets[0], (sizeof(DWORD)* all_file_offset_length));

		// Store info
		DWORD dir_name_ptr = 0;
		for (DWORD dir = 0; dir < tcd2_index_info[file_type].dir_count; dir++)
		{
			// Get folder name
			char dir_name[_MAX_DIR];
			strcpy(dir_name, (char*)&all_dir_names[dir_name_ptr]);
			dir_name_ptr += strlen(dir_name) + 1;

			DWORD file_name_ptr = 0;
			for (DWORD file = 0; file < tcd2_dir_info[dir].file_count; file++)
			{
				// Get file name
				char file_name[_MAX_FNAME];
				strcpy(file_name, (char*)&all_file_names[tcd2_dir_info[dir].file_name_offset + file_name_ptr]);
				file_name_ptr += strlen(file_name) + 1;

				// Folder name + File name + Extension
				TCHAR full_name[_MAX_PATH];
				_stprintf(full_name, _T("%s\\%s%s"), dir_name, file_name, file_exts[file_type].GetString());

				// Store info
				SFileInfo file_info;
				file_info.name = full_name;
				file_info.start = all_file_offsets[tcd2_dir_info[dir].file_offset + file + 0];
				file_info.end = all_file_offsets[tcd2_dir_info[dir].file_offset + file + 1];
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
bool CTCD2::DecompRLE2(void* dst, DWORD dst_size, const void* src, DWORD src_size)
{
	const BYTE* byte_src = (const BYTE*)src;
	BYTE*       byte_dst = (BYTE*)dst;

	const DWORD offset = *(DWORD*)&byte_src[0];
	const DWORD pixel_count = *(DWORD*)&byte_src[4];

	DWORD src_header_ptr = 8;
	DWORD src_data_ptr = offset;
	DWORD dst_ptr = 0;

	while ((src_header_ptr < offset) && (src_data_ptr < src_size) && (dst_ptr < dst_size))
	{
		const BYTE work = byte_src[src_header_ptr++];
		WORD length;

		switch (work)
		{
		case 0: // Fill in 0
			length = byte_src[src_header_ptr++] + 1;
			for (DWORD i = 0; i < length; i++)
			{
				for (DWORD j = 0; j < 4; j++)
				{
					byte_dst[dst_ptr++] = 0x00;
				}
			}
			break;

		case 1: // Alpha value 0xFF
			length = byte_src[src_header_ptr++] + 1;
			for (DWORD i = 0; i < length; i++)
			{
				for (DWORD j = 0; j < 3; j++)
				{
					byte_dst[dst_ptr++] = byte_src[src_data_ptr++];
				}

				byte_dst[dst_ptr++] = 0xFF;
			}
			break;

		default: // Alpha values 0x01~0xFE
			for (DWORD j = 0; j < 3; j++)
			{
				byte_dst[dst_ptr++] = byte_src[src_data_ptr++];
			}
			byte_dst[dst_ptr++] = ~(work - 1);
			break;
		}
	}

	return true;
}
