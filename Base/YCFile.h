#pragma once

class YCFile
{
public:
	enum
	{
		modeCreate          = 0x00000001,
		modeNoTruncate      = 0x00000002,
		modeRead            = 0x00000004,
		modeReadWrite       = 0x00000008,
		modeWrite           = 0x00000010,
		modeNoInherit       = 0x00000020,
		shareDenyNone       = 0x00000040,
		shareDenyRead       = 0x00000080,
		shareDenyWrite      = 0x00000100,
		shareExclusive      = 0x00000200,
		shareCompat         = 0x00000400,
		typeText            = 0x00000800,
		typeBinary          = 0x00001000,
		osNoBuffer          = 0x00002000,
		osWriteThrough      = 0x00004000,
		osRandomAccess      = 0x00008000,
		osSequentialScan    = 0x00010000,
	};

	enum class SeekMode
	{
		begin,
		current,
		end,
	};

	YCFile();
	virtual ~YCFile();

	YCFile(const YCFile&) = delete;
	YCFile& operator=(const YCFile&) = delete;

	YCFile(YCFile&&) = default;
	YCFile& operator=(YCFile&&) = default;

	virtual bool Open(LPCTSTR file_path, u32 open_flags);

	virtual void Close();

	virtual DWORD Read(void* buffer, u32 read_size);
	virtual DWORD Write(const void* buffer, u32 write_size);

	virtual u64 Seek(s64 offset, SeekMode seek_mode);
	virtual u64 SeekHed(s64 offset = 0);
	virtual u64 SeekEnd(s64 offset = 0);
	virtual u64 SeekCur(s64 offset);

	virtual u64 GetPosition();
	virtual u64 GetLength();

	virtual YCString GetFilePath() const;
	virtual YCString GetFileName() const;
	virtual YCString GetFileExt() const;

protected:
	YCString m_file_path;
	YCString m_file_name;
	YCString m_file_extension;

private:
	HANDLE m_file;
};
