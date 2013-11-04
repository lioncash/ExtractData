#include "stdafx.h"
#include "YCMultiFile.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

YCMultiFile::YCMultiFile()
{
	m_dwCurrentFileID = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Destructor

YCMultiFile::~YCMultiFile()
{
	Close();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Specifies the mode to open the file in

BOOL YCMultiFile::Open(
	LPCTSTR				pszPathToFile,					// File path
	UINT				uOpenFlags						// Mode
	)
{
	Close();

	return Add( pszPathToFile, uOpenFlags );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Specifies the mode to open the file in

BOOL YCMultiFile::Add(
	LPCTSTR				pszPathToFile,					// File path
	UINT				uOpenFlags						// Mode
	)
{
	BOOL    bReturn = FALSE;
	YCFile* pclFile = new YCFile();

	if( pclFile->Open( pszPathToFile, uOpenFlags ) )
	{
		// Opened successfully

		m_vtpclFile.push_back( pclFile );

		bReturn = TRUE;
	}
	else
	{
		// Failed to open

		delete pclFile;
	}

	return bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Close File

void YCMultiFile::Close()
{
	for( size_t siCnt = 0 ; siCnt < m_vtpclFile.size() ; siCnt++ )
	{
		m_vtpclFile[siCnt]->Close();

		delete m_vtpclFile[siCnt];
	}

	m_vtpclFile.clear();

	m_dwCurrentFileID = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Read File

DWORD YCMultiFile::Read(
	void*				pvBuffer,						// Buffer
	DWORD				dwReadSize						// Read Size
	)
{
	DWORD dwResult;
	BYTE* pbtBuffer = (BYTE*) pvBuffer;
	DWORD dwBufferPtr = 0;

	while( 1 )
	{
		dwResult = m_vtpclFile[m_dwCurrentFileID]->Read( &pbtBuffer[dwBufferPtr], dwReadSize );

		dwBufferPtr += dwResult;

		if( dwBufferPtr >= dwReadSize )
		{
			// Exit reading

			break;
		}

		// Advance to next file

		SetNextFile();
	}

	return dwBufferPtr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Write File

DWORD YCMultiFile::Write(
	const void*			pvBuffer,						// Buffer
	DWORD				dwWriteSize						// Write Size
	)
{
	return m_vtpclFile[m_dwCurrentFileID]->Write( pvBuffer, dwWriteSize );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Move the file pointer (Seek)

UINT64 YCMultiFile::Seek(
	INT64				n64Offset,						// Number of bytes to seek
	DWORD				dwSeekMode						// Seek Mode
	)
{
	UINT64 u64Position = 0;
	UINT64 u64CurrentPosition;
	UINT64 u64FileSize;

	switch( dwSeekMode )
	{
	case YCFile::begin:
		// From the beginning

		for( size_t siCnt = 0 ; siCnt < m_vtpclFile.size() ; siCnt++ )
		{
			u64FileSize = m_vtpclFile[siCnt]->GetLength();

			if( n64Offset >= u64FileSize )
			{
				n64Offset -= u64FileSize;
				u64Position += u64FileSize;
			}
			else
			{
				SetFile( siCnt );
				break;
			}
		}

		u64Position += m_vtpclFile[m_dwCurrentFileID]->Seek( n64Offset, dwSeekMode );

		break;

	case YCFile::current:
		// From the current position

		if( n64Offset > 0 )
		{
			// Seek in the direction of the position

			for( size_t siCnt = m_dwCurrentFileID ; siCnt < m_vtpclFile.size() ; siCnt++ )
			{
				if( siCnt == m_dwCurrentFileID )
				{
					u64CurrentPosition = m_vtpclFile[siCnt]->GetPosition();
					u64FileSize = m_vtpclFile[siCnt]->GetLength() - u64CurrentPosition;
				}
				else
				{
					u64FileSize = m_vtpclFile[siCnt]->GetLength();
				}

				if( n64Offset >= u64FileSize )
				{
					n64Offset -= u64FileSize;
					u64Position += u64FileSize;
				}
				else
				{
					SetFile( siCnt );
					break;
				}

				dwSeekMode = YCFile::begin;
			}
		}
		else if( n64Offset < 0 )
		{
			// Seek in negative direction

			n64Offset = -n64Offset;

			for( int nCnt = m_nCurrentFileID ; nCnt >= 0 ; nCnt-- )
			{
				if( nCnt == m_nCurrentFileID )
				{
					u64FileSize = m_vtpclFile[nCnt]->GetPosition();
				}
				else
				{
					u64FileSize = m_vtpclFile[nCnt]->GetLength();
				}

				if( n64Offset >= u64FileSize )
				{
					n64Offset -= u64FileSize;
					u64Position += u64FileSize;
				}
				else
				{
					SetFile( nCnt );
					break;
				}

				dwSeekMode = YCFile::end;
			}

			n64Offset = -n64Offset;
		}
		else
		{
			// Do not seek
		}

		u64Position += m_vtpclFile[m_dwCurrentFileID]->Seek( n64Offset, dwSeekMode );

		break;

	case YCFile::end:
		// From the end

		break;
	}

	return u64Position;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Seek from the head of the file

UINT64 YCMultiFile::SeekHed(
	INT64				n64Offset						// Number of bytes to seek
	)
{
	return Seek( n64Offset, YCFile::begin );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Seek from the end of the file

UINT64 YCMultiFile::SeekEnd(
	INT64				n64Offset						// Number of bytes to seek
	)
{
	return Seek( -n64Offset, YCFile::end );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Seek from the current position in the file

UINT64 YCMultiFile::SeekCur(
	INT64				n64Offset						// Number of bytes to seek
	)
{
	return Seek( n64Offset, YCFile::current );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the current file pointer

UINT64 YCMultiFile::GetPosition()
{
	return SeekCur( 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the file length

UINT64 YCMultiFile::GetLength()
{
	UINT64 u64CurrentOffset = GetPosition();
	UINT64 u64EndOffset     = SeekEnd();

	SeekHed( u64CurrentOffset );

	return u64EndOffset;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the file path

YCString YCMultiFile::GetFilePath()
{
	return m_vtpclFile[m_dwCurrentFileID]->GetFilePath();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the file name

YCString YCMultiFile::GetFileName()
{
	return m_vtpclFile[m_dwCurrentFileID]->GetFileName();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the file's extension

YCString YCMultiFile::GetFileExt()
{
	return m_vtpclFile[m_dwCurrentFileID]->GetFileExt();
}

//////////////////////////////////////////////////////////////////////////////////////////
//

void YCMultiFile::SetFile(
	DWORD				dwFileID						// File ID
	)
{
	m_dwCurrentFileID = dwFileID;
}

//////////////////////////////////////////////////////////////////////////////////////////
//

void YCMultiFile::SetFirstFile()
{
	SetFile( 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////
//

void YCMultiFile::SetNextFile()
{
	if( m_dwCurrentFileID >= GetFileCount() )
	{
		// Treat in form of a ring

		SetFile( 0 );
	}
	else
	{
		SetFile( GetCurrentFileID() + 1 );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//

DWORD YCMultiFile::GetCurrentFileID()
{
	return m_dwCurrentFileID;
}

//////////////////////////////////////////////////////////////////////////////////////////
//

size_t YCMultiFile::GetFileCount()
{
	return m_vtpclFile.size();
}
