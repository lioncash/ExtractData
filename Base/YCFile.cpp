
#include	"stdafx.h"
#include	"YCFile.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

YCFile::YCFile()
{
	m_hFile = INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デストラクタ

YCFile::~YCFile()
{
	Close();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	モードを指定して開く

BOOL	YCFile::Open(
	LPCTSTR				pszPathToFile,					// ファイルパス
	UINT				uOpenFlags						// モード
	)
{
	Close();

	if( lstrlen( pszPathToFile ) > MAX_PATH )
	{
		// パスが長すぎる

		return	FALSE;
	}

	// アクセス方法

	DWORD				dwAccess;
	DWORD				dwCreateDisposition;

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

	// 共有モード

	DWORD				dwShare;

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

	// ファイルの属性およびフラグ

	DWORD				dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;

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

	// ファイルが存在するとき、または存在しないときのそれぞれの動作

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

	// ファイルを開く

	m_hFile = ::CreateFile( pszPathToFile, dwAccess, dwShare, NULL, dwCreateDisposition, dwFlagsAndAttributes, NULL );

	// ファイルパスを保持

	m_clsPathToFile = pszPathToFile;
	m_clsFileName = m_clsPathToFile.GetFileName();
	m_clsFileExt = m_clsPathToFile.GetFileExt();

	return	(m_hFile != INVALID_HANDLE_VALUE);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルを閉じる

void	YCFile::Close()
{
	if( m_hFile != INVALID_HANDLE_VALUE )
	{
		::CloseHandle( m_hFile );
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルを読み込む

DWORD	YCFile::Read(
	void*				pvBuffer,						// バッファ
	DWORD				dwReadSize						// 読み込むサイズ
	)
{
	DWORD				dwResult;

	if( !::ReadFile( m_hFile, pvBuffer, dwReadSize, &dwResult, NULL ) )
	{
		dwResult = 0;
	}

	return	dwResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルに書き込む

DWORD	YCFile::Write(
	const void*			pvBuffer,						// バッファ
	DWORD				dwWriteSize						// 書き込むサイズ
	)
{
	DWORD				dwResult;

	if( !::WriteFile( m_hFile, pvBuffer, dwWriteSize, &dwResult, NULL ) )
	{
		dwResult = 0;
	}

	return	dwResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルポインタを移動する

UINT64	YCFile::Seek(
	INT64				n64Offset,						// 移動するバイト数
	DWORD				dwSeekMode						// 移動モード
	)
{
	switch( dwSeekMode )
	{
	case	begin:
		dwSeekMode = FILE_BEGIN;
		break;

	case	current:
		dwSeekMode = FILE_CURRENT;
		break;

	case	end:
		dwSeekMode = FILE_END;
		break;

	default:
		dwSeekMode = FILE_BEGIN;
	}

	LARGE_INTEGER		stliWork;

	stliWork.QuadPart = n64Offset;
	stliWork.LowPart = ::SetFilePointer( m_hFile, stliWork.LowPart, &stliWork.HighPart, dwSeekMode );

	if( (stliWork.LowPart == INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR) )
	{
		// 移動失敗

		stliWork.QuadPart = -1;
	}

	return	(UINT64) (stliWork.QuadPart);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルポインタを先頭から移動する

UINT64	YCFile::SeekHed(
	INT64				n64Offset						// 移動するバイト数
	)
{
	return	Seek( n64Offset, begin );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルポインタを終端から移動する

UINT64	YCFile::SeekEnd(
	INT64				n64Offset						// 移動するバイト数
	)
{
	return	Seek( -n64Offset, end );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルポインタを現在位置から移動する

UINT64	YCFile::SeekCur(
	INT64				n64Offset						// 移動するバイト数
	)
{
	return	Seek( n64Offset, current );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	現在のファイルポインタを取得

UINT64	YCFile::GetPosition()
{
	return	SeekCur( 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルの長さを取得

UINT64	YCFile::GetLength()
{
	UINT64				u64CurrentOffset;
	UINT64				u64EndOffset;

	u64CurrentOffset = GetPosition();

	u64EndOffset = SeekEnd();

	SeekHed( u64CurrentOffset );

	return	u64EndOffset;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルパスを取得

YCString	YCFile::GetFilePath()
{
	return	m_clsPathToFile;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイル名を取得

YCString	YCFile::GetFileName()
{
	return	m_clsFileName;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	拡張子を取得

YCString	YCFile::GetFileExt()
{
	return	m_clsFileExt;
}
