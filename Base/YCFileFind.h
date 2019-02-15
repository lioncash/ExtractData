#pragma once

//----------------------------------------------------------------------------------------
//-- File Search Class -------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class YCFileFind final
{
public:
	YCFileFind();
	~YCFileFind();

	bool FindFirstFile(LPCTSTR file_path);
	bool FindFirstFile(LPCTSTR directory_path, LPCTSTR file_name);
	bool FindNextFile();
	void Close();

	YCString GetFileName() const;
	YCString GetFilePath() const;
	YCString GetFileTitle() const;

	bool IsDirectory() const;
	bool IsDots() const;

private:
	HANDLE          m_find_handle;
	WIN32_FIND_DATA m_find_data;
	YCString        m_path_to_folder;
};
