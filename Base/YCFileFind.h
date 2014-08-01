#pragma once

//----------------------------------------------------------------------------------------
//-- File Search Class -------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class YCFileFind
{
public:
	YCFileFind();
	virtual ~YCFileFind();

	virtual bool FindFirstFile(LPCTSTR pszPathToFile);
	virtual bool FindFirstFile(LPCTSTR pszPathToFolder, LPCTSTR pszFileName);
	virtual bool FindNextFile();
	virtual void Close();

	virtual YCString GetFileName();
	virtual YCString GetFilePath();
	virtual YCString GetFileTitle();

	virtual bool IsDirectory();
	virtual bool IsDots();

private:
	HANDLE          m_hFind;
	WIN32_FIND_DATA m_stwfdFindData;
	YCString        m_clsPathToFolder;
};
