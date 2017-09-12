#include "StdAfx.h"
#include "YCFileFind.h"

/// Constructor
YCFileFind::YCFileFind()
		: m_find_handle{INVALID_HANDLE_VALUE}
{
}

/// Destructor
YCFileFind::~YCFileFind()
{
	Close();
}

/// Start a file search
///
/// @param file_path File path
///
bool YCFileFind::FindFirstFile(LPCTSTR file_path)
{
	// Save directory path
	TCHAR directory_path[_MAX_PATH];
	lstrcpy(directory_path, file_path);
	PathRemoveFileSpec(directory_path);
	m_path_to_folder = directory_path;

	// Start of the file search
	m_find_handle = ::FindFirstFile(file_path, &m_find_data);

	return m_find_handle != INVALID_HANDLE_VALUE;
}

/// Start a file search
///
/// @param directory_path Directory path
/// @param file_name      Filename (can be a wildcard)
///
bool YCFileFind::FindFirstFile(LPCTSTR directory_path, LPCTSTR file_name)
{
	// Save directory path
	TCHAR stripped_directory_path[_MAX_PATH];
	lstrcpy(stripped_directory_path, directory_path);
	PathRemoveBackslash(stripped_directory_path);

	m_path_to_folder = stripped_directory_path;

	// Create a file path
	TCHAR file_path[_MAX_PATH];
	lstrcpy(file_path, stripped_directory_path);
	PathAppend(file_path, file_name);

	// Start of a file search
	m_find_handle = ::FindFirstFile(file_path, &m_find_data);

	return m_find_handle != INVALID_HANDLE_VALUE;
}

/// Continue to search for files
bool YCFileFind::FindNextFile()
{
	return ::FindNextFile(m_find_handle, &m_find_data) != FALSE;
}

/// Close the search file operation
void YCFileFind::Close()
{
	if (m_find_handle != INVALID_HANDLE_VALUE)
	{
		::FindClose(m_find_handle);
		m_find_handle = INVALID_HANDLE_VALUE;
	}
}

/// Gets the name of the found file
YCString YCFileFind::GetFileName() const
{
	return m_find_data.cFileName;
}

/// Gets the file path of the found file
YCString YCFileFind::GetFilePath() const
{
	TCHAR file_path[_MAX_PATH];

	_stprintf(file_path, _T("%s\\%s"), m_path_to_folder.GetString(), m_find_data.cFileName);

	return file_path;
}

/// Gets the title of the found file
YCString YCFileFind::GetFileTitle() const
{
	TCHAR file_title[_MAX_FNAME];

	lstrcpy(file_title, m_find_data.cFileName);
	PathRemoveExtension(file_title);

	return file_title;
}

/// Check whether or not the found 'file' is a directory
bool YCFileFind::IsDirectory() const
{
	return (m_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

/// Examines the marker of the current directory and its parent directory
bool YCFileFind::IsDots() const
{
	return ((lstrcmp(m_find_data.cFileName, _T(".")) == 0) || (lstrcmp(m_find_data.cFileName, _T("..")) == 0));
}
