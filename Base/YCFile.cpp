
#include "stdafx.h"
#include "YCFile.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

YCFile::YCFile()
{
	m_hFile = INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Destructor

YCFile::~YCFile()
{
	Close();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Specifies the mode to open the file with

BOOL YCFile::Open(
	LPCTSTR				pszPathToFile,					// File path
	UINT				uOpenFlags						// Mode
	)
{
	Close();

	if( lstrlen( pszPathToFile ) > MAX_PATH )
	{
		// Path is too long

		return	FALSE;
	}

	// Access method

	DWORD dwAccess;
	DWORD dwCreateDisposition;

	if( uOpenFlags & modeRead )
	{
		dwAccess = GENERIC_READ;
		dwCreateDisposition = OPEN_EXISTING;
	}
	else if( uOpenFlags & modeReadWrite )
	{
		dwAccess = (GENERIC_READ | GENERIC_WRITE);
		dwCreateDisposition = OPEN_EXISTING;
	}
	else if( uOpenFlags & modeWrite )
	{
		dwAccess = GENERIC_WRITE;
		dwCreateDisposition = CREATE_NEW;
	}
	else
	{
		dwAccess = 0;
		dwCreateDisposition = OPEN_EXISTING;
	}

	// Shared mode

	DWORD dwShare;

	if( uOpenFlags & shareDenyNone )
	{
		dwShare = (FILE_SHARE_READ | FILE_SHARE_WRITE);
	}
	else if( uOpenFlags & shareDenyRead )
	{
		dwShare = FILE_SHARE_WRITE;
	}
	else if( uOpenFlags & shareDenyWrite )
	{
		dwShare = FILE_SHARE_READ;
	}
	else
	{
		dwShare = 0;
	}

	// File attributes and flags

	DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;

	if( uOpenFlags & osNoBuffer )
	{
		dwFlagsAndAttributes |= FILE_FLAG_NO_BUFFERING;
	}
	if( uOpenFlags & osWriteThrough )
	{
		dwFlagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
	}
	if( uOpenFlags & osRandomAccess )
	{
		dwFlagsAndAttributes |= FILE_FLAG_RANDOM_ACCESS;
	}
	if( uOpenFlags & osSequentialScan )
	{
		dwFlagsAndAttributes |= FILE_FLAG_SEQUENTIAL_SCAN;
	}

	// If the file exists, check its time. If not, then it does not exist

	if( uOpenFlags & modeCreate )
	{
		if( uOpenFlags & modeNoTruncate )
		{
			dwCreateDisposition = OPEN_ALWAYS;
		}
		else
		{
			dwCreateDisposition = CREATE_ALWAYS;
		}
	}

	// Open the file

	m_hFile = ::CreateFile( pszPathToFile, dwAccess, dwShare, NULL, dwCreateDisposition, dwFlagsAndAttributes, NULL );

	// Holds the file path

	m_clsPathToFile = pszPathToFile;
	m_clsFileName = m_clsPathToFile.GetFileName();
	m_clsFileExt = m_clsPathToFile.GetFileExt();

	return (m_hFile != INVALID_HANDLE_VALUE);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Close File

void YCFile::Close()
{
	if( m_hFile != INVALID_HANDLE_VALUE )
	{
		::CloseHandle( m_hFile );
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Read File

DWORD YCFile::Read(
	void*				pvBuffer,						// Buffer
	DWORD				dwReadSize						// Read size
	)
{
	DWORD dwResult;

	if( !::ReadFile( m_hFile, pvBuffer, dwReadSize, &dwResult, NULL ) )
	{
		dwResult = 0;
	}

	return dwResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Write File

DWORD YCFile::Write(
	const void*			pvBuffer,						// Buffer
	DWORD				dwWriteSize						// Write Size
	)
{
	DWORD dwResult;

	if( !::WriteFile( m_hFile, pvBuffer, dwWriteSize, &dwResult, NULL ) )
	{
		dwResult = 0;
	}

	return dwResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Move the file pointer

UINT64 YCFile::Seek(
	INT64				n64Offset,						// Number of bytes to move
	DWORD				dwSeekMode						// Seek Mode
	)
{
	switch( dwSeekMode )
	{
	case begin:
		dwSeekMode = FILE_BEGIN;
		break;

	case current:
		dwSeekMode = FILE_CURRENT;
		break;

	case end:
		dwSeekMode = FILE_END;
		break;

	default:
		dwSeekMode = FILE_BEGIN;
	}

	LARGE_INTEGER stliWork;

	stliWork.QuadPart = n64Offset;
	stliWork.LowPart = ::SetFilePointer( m_hFile, stliWork.LowPart, &stliWork.HighPart, dwSeekMode );

	if( (stliWork.LowPart == INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR) )
	{
		// Seek fails

		stliWork.QuadPart = -1;
	}

	return (UINT64) (stliWork.QuadPart);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Move file pointer from the head

UINT64 YCFile::SeekHed(
	INT64				n64Offset						// Number of bytes to move
	)
{
	return Seek( n64Offset, begin );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Move the file pointer from the end of the file

UINT64 YCFile::SeekEnd(
	INT64				n64Offset						// Number of bytes to move
	)
{
	return Seek( -n64Offset, end );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Move the file pointer from its current position

UINT64 YCFile::SeekCur(
	INT64				n64Offset						// Number of bytes to move
	)
{
	return Seek( n64Offset, current );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get the current file pointer position

UINT64 YCFile::GetPosition()
{
	return SeekCur( 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the length of the file

UINT64 YCFile::GetLength()
{
	UINT64 u64CurrentOffset = GetPosition();
	UINT64 u64EndOffset     = SeekEnd();

	SeekHed( u64CurrentOffset );

	return u64EndOffset;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the file path

YCString YCFile::GetFilePath()
{
	return m_clsPathToFile;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the file name

YCString YCFile::GetFileName()
{
	return m_clsFileName;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Retrieves the file's extension

YCString YCFile::GetFileExt()
{
	return m_clsFileExt;
}
