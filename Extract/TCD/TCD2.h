#pragma once

#include "Extract/TCD/TCDBase.h"

class CTCD2 final : public CTCDBase
{
public:
	bool Mount(CArcFile* archive) override;

private:
	struct STCD2IndexInfo
	{
		DWORD file_size;             // File size
		DWORD file_count;            // Number of files
		DWORD dir_count;             // Number of folders
		DWORD index_offset;          // Absolute offset value to the index
		DWORD dir_name_length;       // Folder name length
		DWORD file_name_length;      // File name length
		DWORD file_count_ex;         // File count(Extended version)
		DWORD file_name_length_ex;   // File name length(Extended version)
	};

	struct STCD2DirInfo
	{
		DWORD file_count;            // Number of files
		DWORD file_name_offset;      // File name offset value
		DWORD file_offset;           // File offset value
		DWORD reserved;              // Reserved
	};

	bool DecompRLE2(void* dst, DWORD dst_size, const void* src, DWORD src_size) override;
};
