#include "stdafx.h"
#include "Common.h"
#include "File.h"

CFile::CFile()
{
	m_hFile = INVALID_HANDLE_VALUE;
}

CFile::~CFile()
{
	Close();
}

HANDLE CFile::Open(LPCTSTR filename, u32 mode)
{
	if (mode == FILE_READ)
	{
		m_hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	}
	else if (mode == FILE_WRITE)
	{
		m_hFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	}

	return m_hFile;
}

bool CFile::OpenForRead(LPCTSTR filename)
{
	return Open(filename, FILE_READ) != INVALID_HANDLE_VALUE;
}

bool CFile::OpenForWrite(LPCTSTR filename)
{
	return Open(filename, FILE_WRITE) != INVALID_HANDLE_VALUE;
}

void CFile::Close()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

u32 CFile::Read(void* buffer, u32 size)
{
	DWORD read_size;
	ReadFile(m_hFile, buffer, size, &read_size, nullptr);
	return static_cast<u32>(read_size);
}

u32 CFile::ReadLine(void* buffer, u32 buffer_size, bool delete_line_code)
{
	u8* byte_buffer = static_cast<LPBYTE>(buffer);
	const u8* byte_buffer_end = byte_buffer + buffer_size;
	u32 total_read_size = 0;

	while (true)
	{
		// Read one byte
		const u32 read_size = Read(byte_buffer, 1);
		total_read_size += read_size;

		if (*byte_buffer == '\n')
		{
			// Reached newline character

			if (delete_line_code)
			{
				// Remove carriage return character
				if (*(byte_buffer - 1) == '\r')
					*(byte_buffer - 1) = '\0';
				*byte_buffer = '\0';
			}

			break;
		}

		if (read_size == 0)
		{
			// Read until the end of the file (EOF).
			break;
		}

		byte_buffer++;

		if (byte_buffer >= byte_buffer_end)
		{
			// Filled the entire buffer.
			break;
		}
	}

	return total_read_size;
}

u32 CFile::Write(const void* buffer, u32 size)
{
	DWORD write_size;
	WriteFile(m_hFile, buffer, size, &write_size, nullptr);
	return static_cast<u32>(write_size);
}

void CFile::WriteLine(const void* buffer)
{
	const u8* byte_buffer = static_cast<const u8*>(buffer);

	while (true)
	{
		if (*byte_buffer == '\0')
		{
			// Has reached the null termination character
			break;
		}

		// Writing one byte
		const u32 write_size = Write(byte_buffer, 1);

		if (write_size == 0)
		{
			// Could not be written
			break;
		}

		if (*byte_buffer == '\n')
		{
			// Reached newline character
			break;
		}

		byte_buffer++;
	}
}

u64 CFile::Seek(s64 offset, u32 seek_mode)
{
	LARGE_INTEGER li;
	li.QuadPart = offset;
	li.LowPart = SetFilePointer(m_hFile, li.LowPart, &li.HighPart, seek_mode);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
		li.QuadPart = -1;

	return static_cast<u64>(li.QuadPart);
}

u64 CFile::SeekHed(s64 offset)
{
	return Seek(offset, FILE_BEGIN);
}

u64 CFile::SeekEnd(s64 offset)
{
	return Seek(-offset, FILE_END);
}

u64 CFile::SeekCur(s64 offset)
{
	return Seek(offset, FILE_CURRENT);
}

u64 CFile::GetFilePointer()
{
	return Seek(0, FILE_CURRENT);
}

u64 CFile::GetFileSize()
{
	LARGE_INTEGER li = {};
	li.LowPart = ::GetFileSize(m_hFile, &reinterpret_cast<DWORD&>(li.HighPart));
	return static_cast<u64>(li.QuadPart);
}
