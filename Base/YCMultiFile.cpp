#include "stdafx.h"
#include "YCMultiFile.h"

/// Constructor
YCMultiFile::YCMultiFile()
{
	m_dwCurrentFileID = 0;
}

/// Destructor
YCMultiFile::~YCMultiFile()
{
	Close();
}

/// Specifies the mode to open the file in
///
/// @param pszPathToFile File path
/// @param uOpenFlags    Mode
///
BOOL YCMultiFile::Open(LPCTSTR pszPathToFile, UINT uOpenFlags)
{
	Close();

	return Add(pszPathToFile, uOpenFlags);
}

/// Specifies the mode to open the file in
///
/// @param pszPathToFile File path
/// @param uOpenFlags    Mode
///
BOOL YCMultiFile::Add(LPCTSTR pszPathToFile, UINT uOpenFlags)
{
	BOOL bReturn = FALSE;
	auto pclFile = std::make_unique<YCFile>();

	if (pclFile->Open(pszPathToFile, uOpenFlags))
	{
		// Opened successfully

		m_vtpclFile.push_back(std::move(pclFile));

		bReturn = TRUE;
	}

	return bReturn;
}

/// Close File
void YCMultiFile::Close()
{
	for (auto& file : m_vtpclFile)
	{
		file->Close();
	}

	m_vtpclFile.clear();

	m_dwCurrentFileID = 0;
}

/// Read File
///
/// @param pvBuffer   Buffer
/// @param dwReadSize Read size
///
DWORD YCMultiFile::Read(void* pvBuffer, DWORD dwReadSize)
{
	DWORD dwResult;
	BYTE* pbtBuffer = static_cast<BYTE*>(pvBuffer);
	DWORD dwBufferPtr = 0;

	while (true)
	{
		dwResult = m_vtpclFile[m_dwCurrentFileID]->Read(&pbtBuffer[dwBufferPtr], dwReadSize);

		dwBufferPtr += dwResult;

		if (dwBufferPtr >= dwReadSize)
		{
			// Exit reading
			break;
		}

		// Advance to next file
		SetNextFile();
	}

	return dwBufferPtr;
}

/// Write File
///
/// @param pvBuffer    Buffer
/// @param dwWriteSize Write size
///
DWORD YCMultiFile::Write(const void* pvBuffer, DWORD dwWriteSize)
{
	return m_vtpclFile[m_dwCurrentFileID]->Write(pvBuffer, dwWriteSize);
}

/// Move the file pointer (Seek)
///
/// @param n64Offset  Number of bytes to seek
/// @param dwSeekMode Seek mode
///
UINT64 YCMultiFile::Seek(INT64 n64Offset, DWORD dwSeekMode)
{
	UINT64 u64Position = 0;
	UINT64 u64CurrentPosition;
	UINT64 u64FileSize;

	switch (dwSeekMode)
	{
	case YCFile::begin: // From the beginning

		for (size_t siCnt = 0; siCnt < m_vtpclFile.size(); siCnt++)
		{
			u64FileSize = m_vtpclFile[siCnt]->GetLength();

			if (n64Offset >= u64FileSize)
			{
				n64Offset -= u64FileSize;
				u64Position += u64FileSize;
			}
			else
			{
				SetFile(siCnt);
				break;
			}
		}

		u64Position += m_vtpclFile[m_dwCurrentFileID]->Seek(n64Offset, dwSeekMode);

		break;

	case YCFile::current: // From the current position
		if (n64Offset > 0) // Seek forwards
		{
			for (size_t siCnt = m_dwCurrentFileID; siCnt < m_vtpclFile.size(); siCnt++)
			{
				if (siCnt == m_dwCurrentFileID)
				{
					u64CurrentPosition = m_vtpclFile[siCnt]->GetPosition();
					u64FileSize = m_vtpclFile[siCnt]->GetLength() - u64CurrentPosition;
				}
				else
				{
					u64FileSize = m_vtpclFile[siCnt]->GetLength();
				}

				if (n64Offset >= u64FileSize)
				{
					n64Offset -= u64FileSize;
					u64Position += u64FileSize;
				}
				else
				{
					SetFile(siCnt);
					break;
				}

				dwSeekMode = YCFile::begin;
			}
		}
		// Seek in negative direction
		else if (n64Offset < 0)
		{
			n64Offset = -n64Offset;

			for (int nCnt = m_nCurrentFileID; nCnt >= 0; nCnt--)
			{
				if (nCnt == m_nCurrentFileID)
				{
					u64FileSize = m_vtpclFile[nCnt]->GetPosition();
				}
				else
				{
					u64FileSize = m_vtpclFile[nCnt]->GetLength();
				}

				if (n64Offset >= u64FileSize)
				{
					n64Offset -= u64FileSize;
					u64Position += u64FileSize;
				}
				else
				{
					SetFile(nCnt);
					break;
				}

				dwSeekMode = YCFile::end;
			}

			n64Offset = -n64Offset;
		}

		u64Position += m_vtpclFile[m_dwCurrentFileID]->Seek(n64Offset, dwSeekMode);
		break;

	case YCFile::end: // From the end
		break;
	}

	return u64Position;
}

/// Seek from the head of the file
///
/// @param n64Offset Number of bytes to seek
///
UINT64 YCMultiFile::SeekHed(INT64 n64Offset)
{
	return Seek(n64Offset, YCFile::begin);
}

/// Seek from the end of the file
///
/// @param n64Offset Number of bytes to seek
///
UINT64 YCMultiFile::SeekEnd(INT64 n64Offset)
{
	return Seek(-n64Offset, YCFile::end);
}

/// Seek from the current position in the file
///
/// @param n64Offset Number of bytes to seek
///
UINT64 YCMultiFile::SeekCur(INT64 n64Offset)
{
	return Seek(n64Offset, YCFile::current);
}

/// Gets the current file pointer
UINT64 YCMultiFile::GetPosition()
{
	return SeekCur(0);
}

/// Gets the file length
UINT64 YCMultiFile::GetLength()
{
	UINT64 u64CurrentOffset = GetPosition();
	UINT64 u64EndOffset = SeekEnd();

	SeekHed(u64CurrentOffset);

	return u64EndOffset;
}

/// Gets the file path
YCString YCMultiFile::GetFilePath()
{
	return m_vtpclFile[m_dwCurrentFileID]->GetFilePath();
}

/// Gets the file name
YCString YCMultiFile::GetFileName()
{
	return m_vtpclFile[m_dwCurrentFileID]->GetFileName();
}

/// Gets the file's extension
YCString YCMultiFile::GetFileExt()
{
	return m_vtpclFile[m_dwCurrentFileID]->GetFileExt();
}

void YCMultiFile::SetFile(DWORD dwFileID)
{
	m_dwCurrentFileID = dwFileID;
}

void YCMultiFile::SetFirstFile()
{
	SetFile(0);
}

void YCMultiFile::SetNextFile()
{
	if (m_dwCurrentFileID >= GetFileCount())
	{
		// Treat in form of a ring

		SetFile(0);
	}
	else
	{
		SetFile(GetCurrentFileID() + 1);
	}
}

DWORD YCMultiFile::GetCurrentFileID()
{
	return m_dwCurrentFileID;
}

size_t YCMultiFile::GetFileCount()
{
	return m_vtpclFile.size();
}
