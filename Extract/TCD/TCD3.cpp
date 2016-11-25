#include "StdAfx.h"
#include "../../Image.h"
#include "../../Sound/Ogg.h"
#include "TCD3.h"

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
	DWORD files;
	archive->Read(&files, 4);

	// Get index info
	STCD3IndexInfo tcd3_index_info[5];
	archive->Read(tcd3_index_info, (sizeof(STCD3IndexInfo)* 5));

	// Create Key Table
	static constexpr BYTE key[5] = {
		0xB7, 0x39, 0x24, 0x8D, 0x8D
	};

	// Create extension table
	static const YCString file_exts[5] =
	{
		_T(".tct"), _T(".tsf"), _T(".spd"), _T(".ogg"), _T(".wav")
	};

	// Read index
	for (DWORD file_type = 0; file_type < 5; file_type++)
	{
		if (tcd3_index_info[file_type].file_size == 0)
		{
			// Index does not exist
			continue;
		}

		// Go to index
		archive->SeekHed(tcd3_index_info[file_type].index_offset);

		// Folder name
		const DWORD all_dir_name_length = (tcd3_index_info[file_type].dir_name_length * tcd3_index_info[file_type].dir_count);
		YCMemory<BYTE> all_dir_names(all_dir_name_length);
		archive->Read(&all_dir_names[0], all_dir_name_length);

		// Decode folder name
		for (size_t i = 0; i < all_dir_name_length; i++)
		{
			all_dir_names[i] -= key[file_type];
		}

		// Get folder info
		YCMemory<STCD3DirInfo> tcd3_dir_info(tcd3_index_info[file_type].dir_count);
		archive->Read(&tcd3_dir_info[0], (sizeof(STCD3DirInfo)* tcd3_index_info[file_type].dir_count));

		// File name
		const DWORD all_file_name_length = (tcd3_index_info[file_type].file_name_length * tcd3_index_info[file_type].file_count);
		YCMemory<BYTE> all_file_names(all_file_name_length);
		archive->Read(&all_file_names[0], all_file_name_length);

		// Decode file name
		for (size_t i = 0; i < all_file_name_length; i++)
		{
			all_file_names[i] -= key[file_type];
		}

		// File offset
		const DWORD all_file_offset_length = (tcd3_index_info[file_type].file_count + 1);
		YCMemory<DWORD> all_file_offsets(all_file_offset_length);
		archive->Read(&all_file_offsets[0], (sizeof(DWORD)* all_file_offset_length));

		// Store Info
		for (DWORD dir = 0; dir < tcd3_index_info[file_type].dir_count; dir++)
		{
			// Get folder info
			TCHAR dir_name[_MAX_DIR];
			memcpy(dir_name, &all_dir_names[tcd3_index_info[file_type].dir_name_length * dir], tcd3_index_info[file_type].dir_name_length);

			for (DWORD file = 0; file < tcd3_dir_info[dir].file_count; file++)
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
bool CTCD3::DecompRLE2(void* dst, DWORD dst_size, const void* src, DWORD src_size)
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
		const WORD work = *(WORD*)&byte_src[src_header_ptr];

		src_header_ptr += 2;

		const WORD type = work >> 14;
		const WORD length = work & 0x3FFF;

		if ((dst_ptr + (length * 4)) > dst_size)
		{
			MessageBox(nullptr, _T("The output buffer to decompress RLE2 is too small."), _T("Error"), 0);
		}

		switch (type)
		{
		case 0: // Fill in 0
			for (DWORD i = 0; i < length; i++)
			{
				for (DWORD j = 0; j < 4; j++)
				{
					byte_dst[dst_ptr++] = 0x00;
				}
			}
			break;

		case 1:  // Alpha value 0xFF
			if ((src_data_ptr + (length * 3)) > src_size)
			{
				MessageBox(nullptr, _T("Input buffer to decompress RLE2 is too small."), _T("Error"), 0);
			}

			for (DWORD i = 0; i < length; i++)
			{
				for (DWORD j = 0; j < 3; j++)
				{
					byte_dst[dst_ptr++] = byte_src[src_data_ptr++];
				}

				byte_dst[dst_ptr++] = 0xFF;
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

			for (DWORD i = 0; i < length; i++)
			{
				for (DWORD j = 0; j < 3; j++)
				{
					byte_dst[dst_ptr++] = byte_src[src_data_ptr++];
				}

				byte_dst[dst_ptr++] = byte_src[src_header_ptr++];
			}
			break;
		}
	}

	return true;
}
