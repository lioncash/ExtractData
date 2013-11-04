
#include "stdafx.h"
#include "YCFileFindSx.h"

//////////////////////////////////////////////////////////////////////////////////////////
// File Search

BOOL YCFileFindSx::FindFile(
	std::vector<YCString>&	rfvcPathToDstFile,			// Storage location
	LPCTSTR					pszPathToTargetFolder,		// Directory to search for
	LPCTSTR					pszTargetFileName,			// Target filename(Can be a wildcard)
	BOOL					bSearchSubDirectory			// Search for subdirectories (TRUE: Search it, FALSE: Don't search it)
	)
{
	BOOL bReturn = FALSE;

	// Searching for file

	YCFileFind clffTarget;

	if( clffTarget.FindFirstFile( pszPathToTargetFolder, pszTargetFileName ) )
	{
		do
		{
			if( clffTarget.IsDots() )
			{
				// Marker

				continue;
			}

			if( clffTarget.IsDirectory() )
			{
				// Directory

				continue;
			}

			// Add to the list of files found

			rfvcPathToDstFile.push_back( clffTarget.GetFilePath() );

		}
		while( clffTarget.FindNextFile() );

		bReturn = TRUE;
	}

	clffTarget.Close();

	if( !bSearchSubDirectory )
	{
		// Do not search the subdirectories

		return bReturn;
	}

	// Search directory

	if( clffTarget.FindFirstFile( pszPathToTargetFolder, _T("*.*") ) )
	{
		do
		{
			if( clffTarget.IsDots() )
			{
				// Marker

				continue;
			}

			if( !clffTarget.IsDirectory() )
			{
				// Not a directory

				continue;
			}

			// Recursive call

			FindFile( rfvcPathToDstFile, clffTarget.GetFilePath(), pszTargetFileName, bSearchSubDirectory );
		}
		while( clffTarget.FindNextFile() );

		bReturn = TRUE;
	}

	clffTarget.Close();

	return bReturn;
}
