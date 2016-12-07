#pragma once

#include "Extract/TCD/TCDBase.h"

class CTCD3 final : public CTCDBase
{
public:
	bool Mount(CArcFile* archive) override;

private:
	struct STCD3IndexInfo
	{
		u32 file_size;             // File size
		u32 index_offset;          // Absolute offset value to the index
		u32 dir_count;             // Number of folders
		u32 dir_name_length;       // Folder name length (4 byte unites. Example ÅF SYSTEM == 8 bytes)
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

	bool DecompRLE2(u8* dst, size_t dst_size, const u8* src, size_t src_size) override;
};
