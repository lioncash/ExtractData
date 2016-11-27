#pragma once

#include "Extract/TCD/TCDBase.h"

class CTCD3 final : public CTCDBase
{
public:
	bool Mount(CArcFile* archive) override;

private:
	struct STCD3IndexInfo
	{
		DWORD file_size;             // File size
		DWORD index_offset;          // Absolute offset value to the index
		DWORD dir_count;             // Number of folders
		DWORD dir_name_length;       // Folder name length (4 byte unites. Example ÅF SYSTEM == 8 bytes)
		DWORD file_count;            // Number of files
		DWORD file_name_length;      // File name length
		DWORD file_count_ex;         // File count (Extended version)
		DWORD file_name_length_ex;   // File name length (Extended version)
	};

	struct STCD3DirInfo
	{
		DWORD file_count;            // Number of files
		DWORD file_name_offset;      // File name offset value
		DWORD file_offset;           // File offset value
		DWORD reserved;              // Reserved
	};

	bool DecompRLE2(void* dst, DWORD dst_size, const void* src, DWORD src_size) override;
};
