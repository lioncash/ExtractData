#include "StdAfx.h"
#include "Common.h"
#include "FindFile.h"

std::vector<YCString> CFindFile::DoFind(LPCTSTR base_path, LPCTSTR file_name)
{
	HANDLE find_handle;
	WIN32_FIND_DATA work;
	TCHAR base_path_array[MAX_PATH];
	TCHAR file_path_array[MAX_PATH];

//-- File Searching ------------------------------------------------------------------------

	lstrcpy(base_path_array, base_path);
	PathRemoveBackslash(base_path_array);

	_stprintf(file_path_array, _T("%s\\%s"), base_path_array, file_name);

	find_handle = FindFirstFile(file_path_array, &work);

	if (find_handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			// Add to the list of files found
			YCString file_path = base_path_array;
			file_path += _T("\\");
			file_path += work.cFileName;

			m_file_paths.push_back(file_path);
		} while (FindNextFile(find_handle, &work));

		FindClose(find_handle);
	}

//-- Search Directory --------------------------------------------------------------------

	_stprintf(file_path_array, _T("%s\\*.*"), base_path_array); // Search for all files

	find_handle = FindFirstFile(file_path_array, &work);

	if (find_handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(work.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// Is not a folder
				continue;
			}

			if (lstrcmp(work.cFileName, _T(".")) == 0 || lstrcmp(work.cFileName, _T("..")) == 0)
			{
				// Is not current route
				continue;
			}

			// Prepare directory discovery (add to the directory search path discovery)
			_stprintf(file_path_array, _T("%s\\%s"), base_path_array, work.cFileName);

			// Recursive call
			DoFind(file_path_array, file_name);
		} while (FindNextFile(find_handle, &work));

		FindClose(find_handle);
	}

	return m_file_paths;
}

void CFindFile::Clear()
{
	m_file_paths.clear();
}
