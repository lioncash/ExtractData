#pragma once

class CFindFile
{
private:
	// Member Variables

	std::vector<YCString> m_vtsPathToFile;

public:
	// Member Functions

	std::vector<YCString> DoFind(LPCTSTR pszBasePath, LPCTSTR pszFileName);
	void                  Clear();
};
