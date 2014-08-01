#pragma once

class YCFileFindSx
{
public:
	bool FindFile(std::vector<YCString>& rfvcPathToDstFile, LPCTSTR pszPathToTargetFolder, LPCTSTR pszTargetFileName = _T("*.*"), bool bSearchSubDirectory = true);
};
