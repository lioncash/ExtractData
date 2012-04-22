
#include	"stdafx.h"
#include	"YCMultiFile.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

YCMultiFile::YCMultiFile()
{
	m_dwCurrentFileID = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デストラクタ

YCMultiFile::~YCMultiFile()
{
	Close();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	モードを指定して開く

BOOL	YCMultiFile::Open(
	LPCTSTR				pszPathToFile,					// ファイルパス
	UINT				uOpenFlags						// モード
	)
{
	Close();

	return	Add( pszPathToFile, uOpenFlags );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	モードを指定して開く

BOOL	YCMultiFile::Add(
	LPCTSTR				pszPathToFile,					// ファイルパス
	UINT				uOpenFlags						// モード
	)
{
	BOOL				bReturn = FALSE;
	YCFile*				pclFile = new YCFile();

	if( pclFile->Open( pszPathToFile, uOpenFlags ) )
	{
		// オープン成功

		m_vtpclFile.push_back( pclFile );

		bReturn = TRUE;
	}
	else
	{
		// オープン失敗

		delete	pclFile;
	}

	return	bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルを閉じる

void	YCMultiFile::Close()
{
	for( size_t siCnt = 0 ; siCnt < m_vtpclFile.size() ; siCnt++ )
	{
		m_vtpclFile[siCnt]->Close();

		delete	m_vtpclFile[siCnt];
	}

	m_vtpclFile.clear();

	m_dwCurrentFileID = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルを読み込む

DWORD	YCMultiFile::Read(
	void*				pvBuffer,						// バッファ
	DWORD				dwReadSize						// 読み込むサイズ
	)
{
	DWORD				dwResult;
	BYTE*				pbtBuffer = (BYTE*) pvBuffer;
	DWORD				dwBufferPtr = 0;

	while( 1 )
	{
		dwResult = m_vtpclFile[m_dwCurrentFileID]->Read( &pbtBuffer[dwBufferPtr], dwReadSize );

		dwBufferPtr += dwResult;

		if( dwBufferPtr >= dwReadSize )
		{
			// 読み込み終了

			break;
		}

		// 次のファイルを開く

		SetNextFile();
	}

	return	dwBufferPtr;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルに書き込む

DWORD	YCMultiFile::Write(
	const void*			pvBuffer,						// バッファ
	DWORD				dwWriteSize						// 書き込むサイズ
	)
{
	return	m_vtpclFile[m_dwCurrentFileID]->Write( pvBuffer, dwWriteSize );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルポインタを移動する

UINT64	YCMultiFile::Seek(
	INT64				n64Offset,						// 移動するバイト数
	DWORD				dwSeekMode						// 移動モード
	)
{
	UINT64				u64Position = 0;
	UINT64				u64CurrentPosition;
	UINT64				u64FileSize;

	switch( dwSeekMode )
	{
	case	YCFile::begin:
		// 先頭から

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

	case	YCFile::current:
		// 現在位置から

		if( n64Offset > 0 )
		{
			// プラスの方向に移動

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
			// マイナスの方向に移動

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
			// 移動しない


		}

		u64Position += m_vtpclFile[m_dwCurrentFileID]->Seek( n64Offset, dwSeekMode );

		break;

	case	YCFile::end:
		// 終端から





		break;
	}

	return	u64Position;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルポインタを先頭から移動する

UINT64	YCMultiFile::SeekHed(
	INT64				n64Offset						// 移動するバイト数
	)
{
	return	Seek( n64Offset, YCFile::begin );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルポインタを終端から移動する

UINT64	YCMultiFile::SeekEnd(
	INT64				n64Offset						// 移動するバイト数
	)
{
	return	Seek( -n64Offset, YCFile::end );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルポインタを現在位置から移動する

UINT64	YCMultiFile::SeekCur(
	INT64				n64Offset						// 移動するバイト数
	)
{
	return	Seek( n64Offset, YCFile::current );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	現在のファイルポインタを取得

UINT64	YCMultiFile::GetPosition()
{
	return	SeekCur( 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルの長さを取得

UINT64	YCMultiFile::GetLength()
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

YCString	YCMultiFile::GetFilePath()
{
	return	m_vtpclFile[m_dwCurrentFileID]->GetFilePath();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイル名を取得

YCString	YCMultiFile::GetFileName()
{
	return	m_vtpclFile[m_dwCurrentFileID]->GetFileName();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	拡張子を取得

YCString	YCMultiFile::GetFileExt()
{
	return	m_vtpclFile[m_dwCurrentFileID]->GetFileExt();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	

void	YCMultiFile::SetFile(
	DWORD				dwFileID						// ファイルID
	)
{
	m_dwCurrentFileID = dwFileID;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	

void	YCMultiFile::SetFirstFile()
{
	SetFile( 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	

void	YCMultiFile::SetNextFile()
{
	if( m_dwCurrentFileID >= GetFileCount() )
	{
		// リング状に扱う

		SetFile( 0 );
	}
	else
	{
		SetFile( GetCurrentFileID() + 1 );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	

DWORD	YCMultiFile::GetCurrentFileID()
{
	return	m_dwCurrentFileID;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	

size_t	YCMultiFile::GetFileCount()
{
	return	m_vtpclFile.size();
}
