
#include "stdafx.h"
#include "YCFileFind.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

YCFileFind::YCFileFind()
{
	m_hFind = INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Destructor
YCFileFind::~YCFileFind()
{
	Close();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Start a file search
//
// Parameters:
//   - pszPathToFile - File path

BOOL YCFileFind::FindFirstFile(LPCTSTR pszPathToFile)
{
	// Save directory path
	TCHAR szPathToFolder[_MAX_PATH];
	lstrcpy(szPathToFolder, pszPathToFile);
	PathRemoveFileSpec(szPathToFolder);
	m_clsPathToFolder = szPathToFolder;

	// Start of the file search
	m_hFind = ::FindFirstFile(pszPathToFile, &m_stwfdFindData);

	return (m_hFind != INVALID_HANDLE_VALUE);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Start a file search
//
// Parameters:
//   - pszPathToFolder - Directory path
//   - pszFileName     - Filename (can be a wildcard)

BOOL YCFileFind::FindFirstFile(LPCTSTR pszPathToFolder, LPCTSTR pszFileName)
{
	// Save directory path
	TCHAR szPathToFolder[_MAX_PATH];
	lstrcpy(szPathToFolder, pszPathToFolder);
	PathRemoveBackslash(szPathToFolder);

	m_clsPathToFolder = szPathToFolder;

	// Create a file path
	TCHAR szPathToFile[_MAX_PATH];
	lstrcpy(szPathToFile, szPathToFolder);
	PathAppend(szPathToFile, pszFileName);

	// Start of a file search
	m_hFind = ::FindFirstFile(szPathToFile, &m_stwfdFindData);

	return (m_hFind != INVALID_HANDLE_VALUE);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Continue to search for files

BOOL YCFileFind::FindNextFile()
{
	return ::FindNextFile(m_hFind, &m_stwfdFindData);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Close the search file operation

void YCFileFind::Close()
{
	if (m_hFind != INVALID_HANDLE_VALUE)
	{
		::FindClose(m_hFind);
		m_hFind = INVALID_HANDLE_VALUE;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the name of the found file

YCString YCFileFind::GetFileName()
{
	return m_stwfdFindData.cFileName;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the file path of the found file

YCString YCFileFind::GetFilePath()
{
	TCHAR szPathToFile[_MAX_PATH];

	_stprintf( szPathToFile, _T("%s\\%s"), m_clsPathToFolder, m_stwfdFindData.cFileName );

	return szPathToFile;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the title of the found file

YCString YCFileFind::GetFileTitle()
{
	TCHAR szFileTitle[_MAX_FNAME];

	lstrcpy(szFileTitle, m_stwfdFindData.cFileName);
	PathRemoveExtension(szFileTitle);

	return szFileTitle;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Check whether or not the found 'file' is a directory

BOOL YCFileFind::IsDirectory()
{
	return (m_stwfdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Examines the marker of the current directory and its parent directory

BOOL YCFileFind::IsDots()
{
	return ((lstrcmp(m_stwfdFindData.cFileName, _T(".") ) == 0) || (lstrcmp( m_stwfdFindData.cFileName, _T("..") ) == 0));
}
