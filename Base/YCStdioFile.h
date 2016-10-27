#pragma once

#include "YCFile.h"

//----------------------------------------------------------------------------------------
//-- Text File Class ---------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class YCStdioFile : public YCFile
{
public:
	YCStdioFile();
	virtual ~YCStdioFile();

	bool Open(LPCTSTR pszPathToFile, UINT uOpenFlags) override;
	void Close() override;

	DWORD Read(void* pvBuffer, DWORD dwReadSize) override;
	DWORD Write(const void* pvBuffer, DWORD dwWriteSize) override;

	virtual LPTSTR ReadString(LPTSTR pszBuffer, DWORD dwBufferSize);
	virtual BOOL ReadString(YCString& rfclsBuffer);
	virtual void WriteString(LPCTSTR pszBuffer);

	UINT64 Seek(INT64 offset, DWORD SeekMode) override;

private:
	FILE* m_pStream;
};
