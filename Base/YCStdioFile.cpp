#include "StdAfx.h"
#include "YCStdioFile.h"

/// Constructor
YCStdioFile::YCStdioFile()
	: m_stream{nullptr, std::fclose}
{
}

/// Destructor
YCStdioFile::~YCStdioFile() = default;

/// Mode to open the file in
///
/// @param file_path  File path
/// @param open_flags Mode
///
bool YCStdioFile::Open(LPCTSTR file_path, u32 open_flags)
{
	Close();

	if (lstrlen(file_path) >= MAX_PATH)
	{
		// Path is too long
		return false;
	}

	// Accessing Mode
	YCString mode;

	if (open_flags & typeBinary)
	{
		// Binary
		if (open_flags & modeRead)
		{
			mode = _T("rb");
		}

		if (open_flags & modeCreate)
		{
			if (open_flags & modeNoTruncate)
			{
				if (open_flags & modeReadWrite)
				{
					mode = _T("rb+");
				}
				else if (open_flags & modeWrite)
				{
					mode = _T("ab");
				}
			}
			else
			{
				if (open_flags & modeReadWrite)
				{
					mode = _T("wb+");
				}
				else if (open_flags & modeWrite)
				{
					mode = _T("wb");
				}
			}
		}
		else
		{
			if (open_flags & modeReadWrite)
			{
				mode = _T("ab+");
			}
			else if (open_flags & modeWrite)
			{
				mode = _T("ab");
			}
		}
	}
	else // Text
	{
		if (open_flags & modeRead)
		{
			mode = _T("r");
		}

		if (open_flags & modeCreate)
		{
			if (open_flags & modeNoTruncate)
			{
				if (open_flags & modeReadWrite)
				{
					mode = _T("r+");
				}
				else if (open_flags & modeWrite)
				{
					mode = _T("a");
				}
			}
			else
			{
				if (open_flags & modeReadWrite)
				{
					mode = _T("w+");
				}
				else if (open_flags & modeWrite)
				{
					mode = _T("w");
				}
			}
		}
		else
		{
			if (open_flags & modeReadWrite)
			{
				mode = _T("a+");
			}
			else if (open_flags & modeWrite)
			{
				mode = _T("a");
			}
		}
	}

	// Open File

	m_stream = FilePtr{_tfopen(file_path, mode), std::fclose};

	m_file_path = file_path;
	m_file_name = m_file_path.GetFileName();
	m_file_extension = m_file_path.GetFileExt();

	return m_stream != nullptr;
}

/// Close File
void YCStdioFile::Close()
{
	m_stream.reset();
}

/// Read File
///
/// @param buffer    Buffer
/// @param read_size Read size
///
DWORD YCStdioFile::Read(void* buffer, u32 read_size)
{
	return fread(buffer, 1, read_size, m_stream.get());
}

/// Write File
///
/// @param buffer     Buffer
/// @param write_size Write size
///
DWORD YCStdioFile::Write(const void* buffer, u32 write_size)
{
	return fwrite(buffer, 1, write_size, m_stream.get());
}

/// Read a file line
///
/// @param buffer      Buffer
/// @param buffer_size Buffer size
///
LPTSTR YCStdioFile::ReadString(LPTSTR buffer, u32 buffer_size)
{
	return _fgetts(buffer, buffer_size, m_stream.get());
}

/// Read a file line
///
/// @param buffer Buffer
///
/// @remark Gets rid of the newline at the end
///
bool YCStdioFile::ReadString(YCString& buffer)
{
	bool result = false;
	std::array<TCHAR, 1024> szBuffer;

	buffer = _T("");

	while (true)
	{
		if (ReadString(szBuffer.data(), szBuffer.size()) == nullptr)
		{
			// Read until the end
			break;
		}

		buffer += szBuffer.data();

		result = true;

		if (szBuffer[lstrlen(szBuffer.data()) - 1] == _T('\n'))
		{
			// Read until a newline character
			break;
		}
	}

	return result;
}

/// Writes a line into the file
void YCStdioFile::WriteString(LPCTSTR buffer)
{
	_fputts(buffer, m_stream.get());
}

/// Move the file pointer (Seek)
///
/// @param offset    Number of bytes to seek
/// @param seek_mode Seek mode
///
u64 YCStdioFile::Seek(s64 offset, SeekMode seek_mode)
{
	const u32 internal_seek_mode = [seek_mode] {
		switch (seek_mode)
		{
		case SeekMode::begin:
			return SEEK_SET;

		case SeekMode::current:
			return SEEK_CUR;

		case SeekMode::end:
			return SEEK_END;

		default:
			return SEEK_SET;
		}
	}();

	if (_fseeki64(m_stream.get(), offset, internal_seek_mode))
	{
		return static_cast<u64>(_ftelli64(m_stream.get()));
	}

	return 0;
}
