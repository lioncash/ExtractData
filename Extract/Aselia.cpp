#include "StdAfx.h"
#include "Aselia.h"

bool CAselia::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".gd"))
		return false;

	TCHAR dll_path[MAX_PATH];
	lstrcpy(dll_path, archive->GetArcPath());
	PathRenameExtension(dll_path, _T(".dll"));

	if (PathFileExists(dll_path) == FALSE)
		return false;

	// Open DLL File
	CFile dll_file;
	if (dll_file.Open(dll_path, CFile::FILE_READ) == INVALID_HANDLE_VALUE)
		return false;

	// Get filecount
	u32 num_files;
	dll_file.Read(&num_files, sizeof(u32));
	num_files--;

	// Number of files retrieved from the index size
	const size_t index_size = num_files << 3;

	// Get Index
	std::vector<u8> index(index_size);
	dll_file.Read(index.data(), index.size());

	// Get file extension
	YCString file_ext;
	if (memcmp(&archive->GetHed()[4], "OggS", 4) == 0)
	{
		file_ext = _T(".ogg");
	}
	else if (memcmp(&archive->GetHed()[4], "PNG" /*‰PNG" NOTE: is that the actual way it is (‰PNG), or was it an encoding error in this cpp file? */, 4) == 0)
	{
		file_ext = _T(".png");
	}

	const u8* index_ptr = index.data();
	for (u32 i = 0; i < num_files; i++)
	{
		// Get filename
		TCHAR file_name[MAX_PATH];
		_stprintf(file_name, _T("%s_%06u%s"), archive->GetArcName().GetString(), i + 1, file_ext.GetString());

		SFileInfo file_info;
		file_info.start = *reinterpret_cast<const u32*>(&index_ptr[0]);
		file_info.sizeOrg = *reinterpret_cast<const u32*>(&index_ptr[4]);

		// Add to listview
		file_info.name = file_name;
		file_info.sizeCmp = file_info.sizeOrg;
		file_info.end = file_info.start + file_info.sizeOrg;
		archive->AddFileInfo(file_info);

		index_ptr += 8;
	}

	return true;
}
