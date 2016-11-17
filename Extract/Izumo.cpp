#include "stdafx.h"
#include "Izumo.h"

// Function to get file information from IZUMO *.dat files
bool CIzumo::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".dat") || memcmp(archive->GetHed(), "PAK0", 4) != 0)
		return false;

	// Get index size (Starting address of the first file)
	DWORD index_size;
	archive->Seek(4, FILE_BEGIN);
	archive->Read(&index_size, 4);

	// Get number of bytes to be skipped
	DWORD dummy;
	archive->Read(&dummy, 4);
	const DWORD skip = dummy << 3;

	// Get file count
	DWORD num_files;
	archive->Read(&num_files, 4);

	// Skip unknown data
	archive->Seek(skip, FILE_CURRENT);

	// Skip segments
	index_size -= 16 + skip;

	// Get index
	YCMemory<BYTE> index(index_size);
	LPBYTE index_ptr = &index[0];
	archive->Read(index_ptr, index_size);

	// Get filename index
	LPBYTE file_name_index = index_ptr + (num_files << 4);

	// Remove unneeded filenames 
	for (DWORD i = 1; i < dummy; i++)
	{
		file_name_index += file_name_index[0] + 1;
	}

	for (DWORD i = 0; i < num_files; i++)
	{
		// Get file name
		TCHAR file_name[256];
		const BYTE len = *file_name_index++;
		memcpy(file_name, file_name_index, len);
		file_name[len] = _T('\0');

		// Add to list view
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.sizeOrg = *(LPDWORD)&index_ptr[4];
		file_info.sizeCmp = file_info.sizeOrg;
		file_info.start = *(LPDWORD)&index_ptr[0];
		file_info.end = file_info.start + file_info.sizeCmp;
		archive->AddFileInfo(file_info);

		index_ptr += 16;
		file_name_index += len;
	}

	return true;
}
