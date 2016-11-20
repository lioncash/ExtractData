#pragma once

class CFindFile
{
public:
	std::vector<YCString> DoFind(LPCTSTR base_path, LPCTSTR file_name);
	void Clear();

private:
	std::vector<YCString> m_file_paths;
};
