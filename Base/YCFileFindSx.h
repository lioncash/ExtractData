
#pragma	once

class	YCFileFindSx
{
public:

	BOOL									FindFile( std::vector<YCString>& rfvcPathToDstFile, LPCTSTR pszPathToTargetFolder, LPCTSTR pszTargetFileName = _T("*.*"), BOOL bSearchSubDirectory = TRUE );
};
