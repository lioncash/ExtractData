#pragma once

#include "Common.h"

#define FILE_READ  0
#define FILE_WRITE 1

class CFile
{
public:
	CFile();
	virtual ~CFile();

	virtual HANDLE Open(LPCTSTR filename, u32 mode);
	virtual bool OpenForRead(LPCTSTR filename);
	virtual bool OpenForWrite(LPCTSTR filename);
	virtual void Close();
	virtual u32 Read(void* buffer, u32 size);
	virtual u32 ReadLine(void* buffer, u32 buffer_size, bool delete_line_code = false);
	virtual u32 Write(const void* buffer, u32 size);
	virtual void WriteLine(const void* buffer);
	virtual u64 Seek(s64 offset, u32 seek_mode);
	virtual u64 SeekHed(s64 offset = 0);
	virtual u64 SeekEnd(s64 offset = 0);
	virtual u64 SeekCur(s64 offset);
	virtual u64 GetFilePointer();
	virtual u64 GetFileSize();

protected:
	HANDLE m_file;
};
