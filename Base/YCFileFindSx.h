#pragma once

class YCFileFindSx
{
public:
	bool FindFile(std::vector<YCString>& dst_file_paths, LPCTSTR target_folder_path, LPCTSTR target_file_name = _T("*.*"), bool search_subdirectories = true);
};
