
#include	"stdafx.h"
#include	"YCStdioFile.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

YCStdioFile::YCStdioFile()
{
	m_pStream = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デストラクタ

YCStdioFile::~YCStdioFile()
{
	Close();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	モードを指定して開く

BOOL	YCStdioFile::Open(
	LPCTSTR				pszPathToFile,					// ファイルパス
	UINT				uOpenFlags						// モード
	)
{
	Close();

	if( lstrlen( pszPathToFile ) >= MAX_PATH )
	{
		// パスが長すぎる

		return	FALSE;
	}

	// アクセス方法

	YCString			clsMode;

	if( uOpenFlags & typeBinary )
	{
		// バイナリ

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
		// テキスト

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

	// ファイルを開く

	m_pStream = _tfopen( pszPathToFile, clsMode );

	m_clsPathToFile = pszPathToFile;
	m_clsFileName = m_clsPathToFile.GetFileName();
	m_clsFileExt = m_clsPathToFile.GetFileExt();

	return	(m_pStream != NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルを閉じる

void	YCStdioFile::Close()
{
	if( m_pStream != NULL )
	{
		fclose( m_pStream );
		m_pStream = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルを読み込む

DWORD	YCStdioFile::Read(
	void*				pvBuffer,						// バッファ
	DWORD				dwReadSize						// 読み込むサイズ
	)
{
	DWORD				dwResult;

	dwResult = fread( pvBuffer, 1, dwReadSize, m_pStream );

	return	dwResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルに書き込む

DWORD	YCStdioFile::Write(
	const void*			pvBuffer,						// バッファ
	DWORD				dwWriteSize						// 書き込むサイズ
	)
{
	DWORD				dwResult;

	dwResult = fwrite( pvBuffer, 1, dwWriteSize, m_pStream );

	return	dwResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルを1行読み込む

LPTSTR	YCStdioFile::ReadString(
	LPTSTR				pszBuffer,						// バッファ
	DWORD				dwBufferSize					// バッファサイズ
	)
{
	return	_fgetts( pszBuffer, dwBufferSize, m_pStream );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルを1行読み込む
//
//	備考	終端の改行を取り除く

BOOL	YCStdioFile::ReadString(
	YCString&			rfclsBuffer						// バッファ
	)
{
	BOOL				bReturn = FALSE;
	TCHAR				szBuffer[1024];

	rfclsBuffer = _T("");

	while( 1 )
	{
		if( ReadString( szBuffer, _countof( szBuffer ) ) == NULL )
		{
			// 最後まで読み込んだ

			break;
		}

		rfclsBuffer += szBuffer;

		bReturn = TRUE;

		if( szBuffer[lstrlen( szBuffer ) - 1] == _T('\n') )
		{
			// 改行文字まで読み込んだ

			break;
		}
	}

	return	bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルに1行書き込む

void	YCStdioFile::WriteString(
	LPCTSTR				pszBuffer						// バッファ
	)
{
	_fputts( pszBuffer, m_pStream );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルポインタを移動する

UINT64	YCStdioFile::Seek(
	INT64				n64Offset,						// 移動するバイト数
	DWORD				dwSeekMode						// 移動モード
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
