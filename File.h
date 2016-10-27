#pragma once

#include "Common.h"

#define FILE_READ  0
#define FILE_WRITE 1

class CFile
{
protected:
	HANDLE m_hFile;

public:
	CFile();
	virtual ~CFile();

	virtual HANDLE Open(LPCTSTR pFileName, DWORD Mode);
	virtual bool OpenForRead(LPCTSTR pszFileName);
	virtual bool OpenForWrite(LPCTSTR pszFileName);
	virtual void Close();
	virtual DWORD Read(LPVOID buf, DWORD size);
	virtual DWORD ReadLine(LPVOID buf, DWORD dwBufSize, bool delete_line_code = false);
	virtual DWORD Write(LPCVOID buf, DWORD size);
	virtual void WriteLine(LPCVOID buf);
	virtual QWORD Seek(INT64 offset, DWORD SeekMode);
	virtual QWORD SeekHed(INT64 offset = 0);
	virtual QWORD SeekEnd(INT64 offset = 0);
	virtual QWORD SeekCur(INT64 offset);
	virtual QWORD GetFilePointer();
	virtual QWORD GetFileSize();
};
