#include "StdAfx.h"
#include "YCFileFindSx.h"

/// File Search
///
/// @param dst_file_paths        Storage location
/// @param target_folder_path    Directory to search for
/// @param target_file_name      Target filename (can be a wildcard)
/// @param search_subdirectories Search for subdirectories (true: Search, false: Don't search)
///
bool YCFileFindSx::FindFile(std::vector<YCString>& dst_file_paths, LPCTSTR target_folder_path, LPCTSTR target_file_name, bool search_subdirectories)
{
	bool result = false;

	// Searching for file
	YCFileFind target;

	if (target.FindFirstFile(target_folder_path, target_file_name))
	{
		do
		{
			// Marker
			if (target.IsDots())
			{
				continue;
			}

			// Directory
			if (target.IsDirectory())
			{
				continue;
			}

			// Add to the list of files found
			dst_file_paths.push_back(target.GetFilePath());

		}
		while (target.FindNextFile());

		result = true;
	}

	target.Close();

	// Do not search the subdirectories
	if (!search_subdirectories)
	{
		return result;
	}

	// Search directory
	if (target.FindFirstFile(target_folder_path, _T("*.*")))
	{
		do
		{
			// Marker
			if (target.IsDots())
			{
				continue;
			}

			// Not a directory
			if (!target.IsDirectory())
			{
				continue;
			}

			// Recursive call
			FindFile(dst_file_paths, target.GetFilePath(), target_file_name, search_subdirectories);
		}
		while (target.FindNextFile());

		result = true;
	}

	target.Close();

	return result;
}
