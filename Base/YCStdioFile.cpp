#include "stdafx.h"
#include "YCStdioFile.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

YCStdioFile::YCStdioFile()
{
	m_pStream = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Destructor

YCStdioFile::~YCStdioFile()
{
	Close();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Mode to open the file in
//
// Parameters:
//   - pszPathToFile - File path
//   - uOpenFlags    - Mode

BOOL YCStdioFile::Open(LPCTSTR pszPathToFile, UINT uOpenFlags)
{
	Close();

	if (lstrlen(pszPathToFile) >= MAX_PATH)
	{
		// Path is too long

		return FALSE;
	}

	// Accessing Mode
	YCString clsMode;

	if (uOpenFlags & typeBinary)
	{
		// Binary

		if (uOpenFlags & modeRead)
		{
			clsMode = _T("rb");
		}

		if (uOpenFlags & modeCreate)
		{
			if (uOpenFlags & modeNoTruncate)
			{
				if (uOpenFlags & modeReadWrite)
				{
					clsMode = _T("rb+");
				}
				else if (uOpenFlags & modeWrite)
				{
					clsMode = _T("ab");
				}
			}
			else
			{
				if (uOpenFlags & modeReadWrite)
				{
					clsMode = _T("wb+");
				}
				else if (uOpenFlags & modeWrite)
				{
					clsMode = _T("wb");
				}
			}
		}
		else
		{
			if (uOpenFlags & modeReadWrite)
			{
				clsMode = _T("ab+");
			}
			else if (uOpenFlags & modeWrite)
			{
				clsMode = _T("ab");
			}
		}
	}
	else // Text
	{
		if (uOpenFlags & modeRead)
		{
			clsMode = _T("r");
		}

		if (uOpenFlags & modeCreate)
		{
			if (uOpenFlags & modeNoTruncate)
			{
				if (uOpenFlags & modeReadWrite)
				{
					clsMode = _T("r+");
				}
				else if (uOpenFlags & modeWrite)
				{
					clsMode = _T("a");
				}
			}
			else
			{
				if (uOpenFlags & modeReadWrite)
				{
					clsMode = _T("w+");
				}
				else if (uOpenFlags & modeWrite)
				{
					clsMode = _T("w");
				}
			}
		}
		else
		{
			if (uOpenFlags & modeReadWrite)
			{
				clsMode = _T("a+");
			}
			else if (uOpenFlags & modeWrite)
			{
				clsMode = _T("a");
			}
		}
	}

	// Open File

	m_pStream = _tfopen(pszPathToFile, clsMode);

	m_clsPathToFile = pszPathToFile;
	m_clsFileName = m_clsPathToFile.GetFileName();
	m_clsFileExt = m_clsPathToFile.GetFileExt();

	return (m_pStream != nullptr);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Close File

void YCStdioFile::Close()
{
	if (m_pStream != nullptr)
	{
		fclose(m_pStream);
		m_pStream = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Read File
//
// Parameters:
//   - pvBuffer   - Buffer
//   - dwReadSize - Read size

DWORD YCStdioFile::Read(void* pvBuffer, DWORD dwReadSize)
{
	return fread(pvBuffer, 1, dwReadSize, m_pStream);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Write File
//
// Parameters:
//   - pvBuffer    - Buffer
//   - dwWriteSize - Write size

DWORD YCStdioFile::Write(const void* pvBuffer, DWORD dwWriteSize)
{
	return fwrite(pvBuffer, 1, dwWriteSize, m_pStream);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Read a file line
//
// Parameters:
//   - pszBuffer    - Buffer
//   - dwBufferSize - Buffer size

LPTSTR YCStdioFile::ReadString(LPTSTR pszBuffer, DWORD dwBufferSize)
{
	return _fgetts( pszBuffer, dwBufferSize, m_pStream );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Read a file line
//
// Remark: Gets rid of the newline at the end
//
// Parameters:
//   - rfclsBuffer - Buffer

BOOL YCStdioFile::ReadString(YCString& rfclsBuffer)
{
	BOOL  bReturn = FALSE;
	TCHAR szBuffer[1024];

	rfclsBuffer = _T("");

	while (true)
	{
		if (ReadString(szBuffer, _countof(szBuffer)) == nullptr)
		{
			// Read until the end

			break;
		}

		rfclsBuffer += szBuffer;

		bReturn = TRUE;

		if (szBuffer[lstrlen(szBuffer) - 1] == _T('\n'))
		{
			// Read until a newline character

			break;
		}
	}

	return bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Writes a line into the file

void YCStdioFile::WriteString(LPCTSTR pszBuffer)
{
	_fputts(pszBuffer, m_pStream);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Move the file pointer (Seek)
//
// Parameters:
//   - n64Offset  - Number of bytes to seek
//   - dwSeekMode - Seek mode

UINT64 YCStdioFile::Seek(INT64 n64Offset, DWORD dwSeekMode)
{
	switch (dwSeekMode)
	{
	case begin:
		dwSeekMode = SEEK_SET;
		break;

	case current:
		dwSeekMode = SEEK_CUR;
		break;

	case end:
		dwSeekMode = SEEK_END;
		break;

	default:
		dwSeekMode = SEEK_SET;
	}

	if (_fseeki64(m_pStream, n64Offset, dwSeekMode))
	{
		return (UINT64)_ftelli64(m_pStream);
	}

	return 0;
}
