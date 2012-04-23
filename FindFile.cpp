#include "stdafx.h"
#include "Common.h"
#include "FindFile.h"

std::vector<YCString> CFindFile::DoFind(LPCTSTR pszBasePath, LPCTSTR pszFileName)
{
    HANDLE hFind;
    WIN32_FIND_DATA stwfdWork;
    TCHAR szBasePath[MAX_PATH];
    TCHAR szPathToFile[MAX_PATH];

//-- File Searching ------------------------------------------------------------------------

    lstrcpy(szBasePath, pszBasePath);
    PathRemoveBackslash(szBasePath);

    _stprintf(szPathToFile, _T("%s\\%s"), szBasePath, pszFileName);

    hFind = FindFirstFile(szPathToFile, &stwfdWork);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            // Add to the list of files found
            YCString sPathToFile = szBasePath;
            sPathToFile += _T("\\");
            sPathToFile += stwfdWork.cFileName;

            m_vtsPathToFile.push_back(sPathToFile);
        } while (FindNextFile(hFind, &stwfdWork));

        FindClose(hFind);
    }

//-- Search Directory --------------------------------------------------------------------

    _stprintf(szPathToFile, _T("%s\\*.*"), szBasePath); // Search for all files

    hFind = FindFirstFile(szPathToFile, &stwfdWork);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(stwfdWork.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // Is not a folder
                continue;
            }

            if (lstrcmp(stwfdWork.cFileName, _T(".")) == 0 || lstrcmp(stwfdWork.cFileName, _T("..")) == 0)
            {
                // Is not current route
                continue;
            }

            // Prepare directory discovery (add to the directory search path discovery)
            _stprintf(szPathToFile, _T("%s\\%s"), szBasePath, stwfdWork.cFileName);

            // Recursive call
            DoFind(szPathToFile, pszFileName);
        } while (FindNextFile(hFind, &stwfdWork));

        FindClose(hFind);
    }

    return m_vtsPathToFile;
}

void CFindFile::Clear()
{
    m_vtsPathToFile.clear();
}
