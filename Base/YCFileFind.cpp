
#include	"stdafx.h"
#include	"YCFileFind.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

YCFileFind::YCFileFind()
{
	m_hFind = INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デストラクタ

YCFileFind::~YCFileFind()
{
	Close();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイル検索を開始

BOOL	YCFileFind::FindFirstFile(
	LPCTSTR				pszPathToFile					// ファイルパス
	)
{
	// ディレクトリパスの保存

	TCHAR				szPathToFolder[_MAX_PATH];

	lstrcpy( szPathToFolder, pszPathToFile );
	PathRemoveFileSpec( szPathToFolder );

	m_clsPathToFolder = szPathToFolder;

	// ファイル検索の開始

	m_hFind = ::FindFirstFile( pszPathToFile, &m_stwfdFindData );

	return	(m_hFind != INVALID_HANDLE_VALUE);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイル検索を開始

BOOL	YCFileFind::FindFirstFile(
	LPCTSTR				pszPathToFolder,				// ディレクトリパス
	LPCTSTR				pszFileName						// ファイル名(ワイルドカード可)
	)
{
	// ディレクトリパスの保存

	TCHAR				szPathToFolder[_MAX_PATH];

	lstrcpy( szPathToFolder, pszPathToFolder );
	PathRemoveBackslash( szPathToFolder );	

	m_clsPathToFolder = szPathToFolder;

	// ファイルパスの作成

	TCHAR				szPathToFile[_MAX_PATH];

	lstrcpy( szPathToFile, szPathToFolder );
	PathAppend( szPathToFile, pszFileName );

	// ファイル検索の開始

	m_hFind = ::FindFirstFile( szPathToFile, &m_stwfdFindData );

	return	(m_hFind != INVALID_HANDLE_VALUE);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイル検索を続行

BOOL	YCFileFind::FindNextFile()
{
	return	::FindNextFile( m_hFind, &m_stwfdFindData );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイル検索を終了

void	YCFileFind::Close()
{
	if( m_hFind != INVALID_HANDLE_VALUE )
	{
		::FindClose( m_hFind );
		m_hFind = INVALID_HANDLE_VALUE;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	検索したファイル名を取得

YCString	YCFileFind::GetFileName()
{
	return	m_stwfdFindData.cFileName;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	検索したファイルパスを取得

YCString	YCFileFind::GetFilePath()
{
	TCHAR				szPathToFile[_MAX_PATH];

	_stprintf( szPathToFile, _T("%s\\%s"), m_clsPathToFolder, m_stwfdFindData.cFileName );

	return	szPathToFile;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	検索したファイルタイトルを取得

YCString	YCFileFind::GetFileTitle()
{
	TCHAR				szFileTitle[_MAX_FNAME];

	lstrcpy( szFileTitle, m_stwfdFindData.cFileName );
	PathRemoveExtension( szFileTitle );

	return	szFileTitle;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	検索したファイルがディレクトリかどうかを確認

BOOL	YCFileFind::IsDirectory()
{
	return	(m_stwfdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	現在のディレクトリとその親ディレクトリのマーカーを調べる

BOOL	YCFileFind::IsDots()
{
	return	((lstrcmp( m_stwfdFindData.cFileName, _T(".") ) == 0) || (lstrcmp( m_stwfdFindData.cFileName, _T("..") ) == 0));
}
