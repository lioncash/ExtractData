#pragma once

class CFindFile
{
public:
	std::vector<YCString> DoFind(LPCTSTR pszBasePath, LPCTSTR pszFileName);
	void Clear();

private:
	std::vector<YCString> m_vtsPathToFile;
};
