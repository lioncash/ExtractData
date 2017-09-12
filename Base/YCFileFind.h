#pragma once

//----------------------------------------------------------------------------------------
//-- File Search Class -------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class YCFileFind
{
public:
	YCFileFind();
	virtual ~YCFileFind();

	virtual bool FindFirstFile(LPCTSTR file_path);
	virtual bool FindFirstFile(LPCTSTR directory_path, LPCTSTR file_name);
	virtual bool FindNextFile();
	virtual void Close();

	virtual YCString GetFileName() const;
	virtual YCString GetFilePath() const;
	virtual YCString GetFileTitle() const;

	virtual bool IsDirectory() const;
	virtual bool IsDots() const;

private:
	HANDLE          m_find_handle;
	WIN32_FIND_DATA m_find_data;
	YCString        m_path_to_folder;
};
