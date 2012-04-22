#pragma	once

//----------------------------------------------------------------------------------------
//-- File Search Class -------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class	YCFileFind
{
public:

											YCFileFind();
	virtual									~YCFileFind();

	virtual	BOOL							FindFirstFile( LPCTSTR pszPathToFile );
	virtual	BOOL							FindFirstFile( LPCTSTR pszPathToFolder, LPCTSTR pszFileName );
	virtual	BOOL							FindNextFile();
	virtual	void							Close();

	virtual	YCString						GetFileName();
	virtual	YCString						GetFilePath();
	virtual	YCString						GetFileTitle();

	virtual	BOOL							IsDirectory();
	virtual	BOOL							IsDots();


private:

	HANDLE									m_hFind;
	WIN32_FIND_DATA							m_stwfdFindData;
	YCString								m_clsPathToFolder;
};
