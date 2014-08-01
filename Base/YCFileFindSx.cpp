
#include "stdafx.h"
#include "YCFileFindSx.h"

//////////////////////////////////////////////////////////////////////////////////////////
// File Search
//
// Parameters:
//   - rfvcPathToDstFile     - Storage location
//   - pszPathToTargetFolder - Directory to search for
//   - pszTargetFileName     - Target filename (can be a wildcard)
//   - bSearchSubDirectory   - Search for subdirectories (TRUE: Search it, FALSE: Don't search it)

bool YCFileFindSx::FindFile(std::vector<YCString>& rfvcPathToDstFile, LPCTSTR pszPathToTargetFolder, LPCTSTR pszTargetFileName, bool bSearchSubDirectory)
{
	bool bReturn = false;

	// Searching for file
	YCFileFind clffTarget;

	if (clffTarget.FindFirstFile(pszPathToTargetFolder, pszTargetFileName))
	{
		do
		{
			// Marker
			if (clffTarget.IsDots())
			{
				continue;
			}

			// Directory
			if (clffTarget.IsDirectory())
			{
				continue;
			}

			// Add to the list of files found
			rfvcPathToDstFile.push_back(clffTarget.GetFilePath());

		}
		while (clffTarget.FindNextFile());

		bReturn = true;
	}

	clffTarget.Close();

	// Do not search the subdirectories
	if (!bSearchSubDirectory)
	{
		return bReturn;
	}

	// Search directory
	if (clffTarget.FindFirstFile(pszPathToTargetFolder, _T("*.*")))
	{
		do
		{
			// Marker
			if (clffTarget.IsDots())
			{
				continue;
			}

			// Not a directory
			if (!clffTarget.IsDirectory())
			{
				continue;
			}

			// Recursive call
			FindFile(rfvcPathToDstFile, clffTarget.GetFilePath(), pszTargetFileName, bSearchSubDirectory);
		}
		while (clffTarget.FindNextFile());

		bReturn = true;
	}

	clffTarget.Close();

	return bReturn;
}
