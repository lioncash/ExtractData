#include	"stdafx.h"
#include	"YCStdioFile.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	Constructor

YCStdioFile::YCStdioFile()
{
	m_pStream = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Destructor

YCStdioFile::~YCStdioFile()
{
	Close();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Mode to open the file in

BOOL	YCStdioFile::Open(
	LPCTSTR				pszPathToFile,					// File path
	UINT				uOpenFlags						// Mode
	)
{
	Close();

	if( lstrlen( pszPathToFile ) >= MAX_PATH )
	{
		// Path is too long

		return	FALSE;
	}

	// Accessing Mode

	YCString			clsMode;

	if( uOpenFlags & typeBinary )
	{
		// Binary

		if( uOpenFlags & modeRead )
		{
			clsMode = _T("rb");
		}

		if( uOpenFlags & modeCreate )
		{
			if( uOpenFlags & modeNoTruncate )
			{
				if( uOpenFlags & modeReadWrite )
				{
					clsMode = _T("rb+");
				}
				else if( uOpenFlags & modeWrite )
				{
					clsMode = _T("ab");
				}
			}
			else
			{
				if( uOpenFlags & modeReadWrite )
				{
					clsMode = _T("wb+");
				}
				else if( uOpenFlags & modeWrite )
				{
					clsMode = _T("wb");
				}
			}
		}
		else
		{
			if( uOpenFlags & modeReadWrite )
			{
				clsMode = _T("ab+");
			}
			else if( uOpenFlags & modeWrite )
			{
				clsMode = _T("ab");
			}
		}
	}
	else
	{
		// Text

		if( uOpenFlags & modeRead )
		{
			clsMode = _T("r");
		}

		if( uOpenFlags & modeCreate )
		{
			if( uOpenFlags & modeNoTruncate )
			{
				if( uOpenFlags & modeReadWrite )
				{
					clsMode = _T("r+");
				}
				else if( uOpenFlags & modeWrite )
				{
					clsMode = _T("a");
				}
			}
			else
			{
				if( uOpenFlags & modeReadWrite )
				{
					clsMode = _T("w+");
				}
				else if( uOpenFlags & modeWrite )
				{
					clsMode = _T("w");
				}
			}
		}
		else
		{
			if( uOpenFlags & modeReadWrite )
			{
				clsMode = _T("a+");
			}
			else if( uOpenFlags & modeWrite )
			{
				clsMode = _T("a");
			}
		}
	}

	// Open File

	m_pStream = _tfopen( pszPathToFile, clsMode );

	m_clsPathToFile = pszPathToFile;
	m_clsFileName = m_clsPathToFile.GetFileName();
	m_clsFileExt = m_clsPathToFile.GetFileExt();

	return	(m_pStream != NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Close File

void	YCStdioFile::Close()
{
	if( m_pStream != NULL )
	{
		fclose( m_pStream );
		m_pStream = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Read File

DWORD	YCStdioFile::Read(
	void*				pvBuffer,						// Buffer
	DWORD				dwReadSize						// Read Size
	)
{
	DWORD				dwResult;

	dwResult = fread( pvBuffer, 1, dwReadSize, m_pStream );

	return	dwResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Write File

DWORD	YCStdioFile::Write(
	const void*			pvBuffer,						// Buffer
	DWORD				dwWriteSize						// Write Size
	)
{
	DWORD				dwResult;

	dwResult = fwrite( pvBuffer, 1, dwWriteSize, m_pStream );

	return	dwResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Read a file line

LPTSTR	YCStdioFile::ReadString(
	LPTSTR				pszBuffer,						// Buffer
	DWORD				dwBufferSize					// Buffer Size
	)
{
	return	_fgetts( pszBuffer, dwBufferSize, m_pStream );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Read a file line
//
//	Remark: Gets rid of the newline at the end

BOOL	YCStdioFile::ReadString(
	YCString&			rfclsBuffer						// Buffer
	)
{
	BOOL				bReturn = FALSE;
	TCHAR				szBuffer[1024];

	rfclsBuffer = _T("");

	while( 1 )
	{
		if( ReadString( szBuffer, _countof( szBuffer ) ) == NULL )
		{
			// Read until the end

			break;
		}

		rfclsBuffer += szBuffer;

		bReturn = TRUE;

		if( szBuffer[lstrlen( szBuffer ) - 1] == _T('\n') )
		{
			// Read until a newline character

			break;
		}
	}

	return	bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Writes a line into the file

void	YCStdioFile::WriteString(
	LPCTSTR				pszBuffer						// Buffer
	)
{
	_fputts( pszBuffer, m_pStream );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Move the file pointer (Seek)

UINT64	YCStdioFile::Seek(
	INT64				n64Offset,						// Number of bytes to seek
	DWORD				dwSeekMode						// Seek Mode
	)
{
	switch( dwSeekMode )
	{
	case	begin:
		dwSeekMode = SEEK_SET;
		break;

	case	current:
		dwSeekMode = SEEK_CUR;
		break;

	case	end:
		dwSeekMode = SEEK_END;
		break;

	default:
		dwSeekMode = SEEK_SET;
	}

	if( _fseeki64( m_pStream, n64Offset, dwSeekMode ) )
	{
		return	(UINT64) _ftelli64( m_pStream );
	}

	return	0;
}
