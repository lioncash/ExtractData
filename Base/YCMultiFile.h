#pragma once

#include "YCFile.h"

//----------------------------------------------------------------------------------------
//-- MultiFile Class ---------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class YCMultiFile
{
public:
	YCMultiFile();
	virtual ~YCMultiFile();

	virtual BOOL Open(LPCTSTR pszPathToFile, UINT uOpenFlags);
	virtual BOOL Add(LPCTSTR pszPathToFile, UINT uOpenFlags);

	virtual void Close();

	virtual DWORD Read(void* pvBuffer, DWORD dwReadSize);
	virtual DWORD Write(const void* pvBuffer, DWORD dwWriteSize);

	virtual UINT64 Seek(INT64 n64Offset, DWORD dwSeekMode);
	virtual UINT64 SeekHed(INT64 n64Offset = 0);
	virtual UINT64 SeekEnd(INT64 n64Offset = 0);
	virtual UINT64 SeekCur(INT64 n64Offset);

	virtual UINT64 GetPosition();
	virtual UINT64 GetLength();

	virtual YCString GetFilePath();
	virtual YCString GetFileName();
	virtual YCString GetFileExt();

	void SetFile(DWORD dwFileID);
	void SetFirstFile();
	void SetNextFile();
	DWORD GetCurrentFileID();
	size_t GetFileCount();

private:
	std::vector<std::unique_ptr<YCFile>> m_vtpclFile;
	int m_nCurrentFileID;
	DWORD m_dwCurrentFileID;
};
