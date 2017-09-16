#include "StdAfx.h"
#include "YCFile.h"

/// Constructor
YCFile::YCFile()
{
	m_file = INVALID_HANDLE_VALUE;
}

/// Destructor
YCFile::~YCFile()
{
	Close();
}

/// Specifies the mode to open the file with
///
/// @param file_path  File path
/// @param open_flags Mode
///
bool YCFile::Open(LPCTSTR file_path, u32 open_flags)
{
	Close();

	// Path is too long
	if (lstrlen(file_path) > MAX_PATH)
	{
		return false;
	}

	// Access method

	u32 access;
	u32 create_disposition;

	if (open_flags & modeRead)
	{
		access = GENERIC_READ;
		create_disposition = OPEN_EXISTING;
	}
	else if (open_flags & modeReadWrite)
	{
		access = (GENERIC_READ | GENERIC_WRITE);
		create_disposition = OPEN_EXISTING;
	}
	else if (open_flags & modeWrite)
	{
		access = GENERIC_WRITE;
		create_disposition = CREATE_NEW;
	}
	else
	{
		access = 0;
		create_disposition = OPEN_EXISTING;
	}

	// Shared mode
	u32 share;

	if (open_flags & shareDenyNone)
	{
		share = (FILE_SHARE_READ | FILE_SHARE_WRITE);
	}
	else if (open_flags & shareDenyRead)
	{
		share = FILE_SHARE_WRITE;
	}
	else if (open_flags & shareDenyWrite)
	{
		share = FILE_SHARE_READ;
	}
	else
	{
		share = 0;
	}

	// File attributes and flags

	u32 flags_and_attributes = FILE_ATTRIBUTE_NORMAL;

	if (open_flags & osNoBuffer)
	{
		flags_and_attributes |= FILE_FLAG_NO_BUFFERING;
	}
	if (open_flags & osWriteThrough)
	{
		flags_and_attributes |= FILE_FLAG_WRITE_THROUGH;
	}
	if (open_flags & osRandomAccess)
	{
		flags_and_attributes |= FILE_FLAG_RANDOM_ACCESS;
	}
	if (open_flags & osSequentialScan)
	{
		flags_and_attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
	}

	// If the file exists, check its time. If not, then it does not exist
	if (open_flags & modeCreate)
	{
		if (open_flags & modeNoTruncate)
		{
			create_disposition = OPEN_ALWAYS;
		}
		else
		{
			create_disposition = CREATE_ALWAYS;
		}
	}

	// Open the file
	m_file = ::CreateFile(file_path, access, share, nullptr, create_disposition, flags_and_attributes, nullptr);

	// Holds the file path
	m_file_path = file_path;
	m_file_name = m_file_path.GetFileName();
	m_file_extension = m_file_path.GetFileExt();

	return m_file != INVALID_HANDLE_VALUE;
}

/// Close File
void YCFile::Close()
{
	if (m_file != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_file);
		m_file = INVALID_HANDLE_VALUE;
	}
}

/// Read File
///
/// @param buffer   Buffer
/// @param read_size Number of bytes to read.
///
DWORD YCFile::Read(void* buffer, u32 read_size)
{
	DWORD result;

	if (!::ReadFile(m_file, buffer, read_size, &result, nullptr))
	{
		result = 0;
	}

	return result;
}

/// Write File
///
/// @param buffer     Buffer
/// @param write_size Number of bytes to write.
///
DWORD YCFile::Write(const void* buffer, u32 write_size)
{
	DWORD result;

	if (!::WriteFile(m_file, buffer, write_size, &result, nullptr))
	{
		result = 0;
	}

	return result;
}

/// Move the file pointer
///
/// @param offset    Number of bytes to move
/// @param seek_mode Seek mode
///
u64 YCFile::Seek(s64 offset, SeekMode seek_mode)
{
	const u32 internal_seek_mode = [seek_mode] {
		switch (seek_mode)
		{
		case SeekMode::begin:
			return FILE_BEGIN;

		case SeekMode::current:
			return FILE_CURRENT;

		case SeekMode::end:
			return FILE_END;

		default:
			return FILE_BEGIN;
		}
	}();

	LARGE_INTEGER work;
	work.QuadPart = offset;
	work.LowPart = ::SetFilePointer(m_file, work.LowPart, &work.HighPart, internal_seek_mode);

	if (work.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		// Seek fails

		work.QuadPart = -1;
	}

	return static_cast<u64>(work.QuadPart);
}

// /Move file relative to the start of the file
///
/// @param offset Number of bytes to seek
///
u64 YCFile::SeekHed(s64 offset)
{
	return Seek(offset, SeekMode::begin);
}

/// Move the file pointer relative to the end of the file
///
/// @param offset - Number of bytes to seek
///
u64 YCFile::SeekEnd(s64 offset)
{
	return Seek(-offset, SeekMode::end);
}

/// Move the file pointer relative to its current position
///
/// @param offset Number of bytes to seek
///
u64 YCFile::SeekCur(s64 offset)
{
	return Seek(offset, SeekMode::current);
}

/// Get the current file pointer position
u64 YCFile::GetPosition()
{
	return SeekCur(0);
}

/// Gets the length of the file
u64 YCFile::GetLength()
{
	const u64 current_offset = GetPosition();
	const u64 end_offset     = SeekEnd();

	SeekHed(current_offset);

	return end_offset;
}

/// Gets the file path
YCString YCFile::GetFilePath() const
{
	return m_file_path;
}

/// Gets the file name
YCString YCFile::GetFileName() const
{
	return m_file_name;
}

/// Retrieves the file's extension
YCString YCFile::GetFileExt() const
{
	return m_file_extension;
}
