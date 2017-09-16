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

	bool Open(LPCTSTR file_path, u32 open_flags) override;
	void Close() override;

	DWORD Read(void* buffer, u32 read_size) override;
	DWORD Write(const void* buffer, u32 write_size) override;

	virtual LPTSTR ReadString(LPTSTR buffer, u32 buffer_size);
	virtual bool ReadString(YCString& buffer);
	virtual void WriteString(LPCTSTR buffer);

	u64 Seek(s64 offset, SeekMode seek_mode) override;

private:
	FILE* m_stream = nullptr;
};
