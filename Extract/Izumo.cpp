#include "StdAfx.h"
#include "Extract/Izumo.h"

#include "ArcFile.h"

// Function to get file information from IZUMO *.dat files
bool CIzumo::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".dat") || memcmp(archive->GetHeader(), "PAK0", 4) != 0)
		return false;

	// Get index size (Starting address of the first file)
	u32 index_size;
	archive->Seek(4, FILE_BEGIN);
	archive->ReadU32(&index_size);

	// Get number of bytes to be skipped
	u32 dummy;
	archive->ReadU32(&dummy);
	const u32 skip = dummy << 3;

	// Get file count
	u32 num_files;
	archive->ReadU32(&num_files);

	// Skip unknown data
	archive->Seek(skip, FILE_CURRENT);

	// Skip segments
	index_size -= 16 + skip;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());
	const u8* index_ptr = index.data();

	// Get filename index
	const u8* file_name_index = index_ptr + (num_files << 4);

	// Remove unneeded filenames 
	for (u32 i = 1; i < dummy; i++)
	{
		file_name_index += file_name_index[0] + 1;
	}

	for (u32 i = 0; i < num_files; i++)
	{
		// Get file name
		TCHAR file_name[256];
		const u8 len = *file_name_index++;
		memcpy(file_name, file_name_index, len);
		file_name[len] = _T('\0');

		// Add to list view
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.size_org = *reinterpret_cast<const u32*>(&index_ptr[4]);
		file_info.size_cmp = file_info.size_org;
		file_info.start = *reinterpret_cast<const u32*>(&index_ptr[0]);
		file_info.end = file_info.start + file_info.size_cmp;
		archive->AddFileInfo(file_info);

		index_ptr += 16;
		file_name_index += len;
	}

	return true;
}
