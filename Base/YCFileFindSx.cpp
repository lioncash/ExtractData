
#include	"stdafx.h"
#include	"YCFileFindSx.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	ファイル検索

BOOL	YCFileFindSx::FindFile(
	std::vector<YCString>&	rfvcPathToDstFile,			// 格納先
	LPCTSTR					pszPathToTargetFolder,		// 検索対象ディレクトリ
	LPCTSTR					pszTargetFileName,			// 該当ファイル名(ワイルドカード可)
	BOOL					bSearchSubDirectory			// サブディレクトリの検索指定(TRUE：検索する、FALSE：検索しない)
	)
{
	BOOL				bReturn = FALSE;

	// ファイル探索

	YCFileFind			clffTarget;

	if( clffTarget.FindFirstFile( pszPathToTargetFolder, pszTargetFileName ) )
	{
		do
		{
			if( clffTarget.IsDots() )
			{
				// マーカー

				continue;
			}

			if( clffTarget.IsDirectory() )
			{
				// ディレクトリ

				continue;
			}

			// 見つかったファイルをリストに加える

			rfvcPathToDstFile.push_back( clffTarget.GetFilePath() );

		}
		while( clffTarget.FindNextFile() );

		bReturn = TRUE;
	}

	clffTarget.Close();

	if( !bSearchSubDirectory )
	{
		// サブディレクトリを検索しない

		return	bReturn;
	}

	// ディレクトリ探索

	if( clffTarget.FindFirstFile( pszPathToTargetFolder, _T("*.*") ) )
	{
		do
		{
			if( clffTarget.IsDots() )
			{
				// マーカー

				continue;
			}

			if( !clffTarget.IsDirectory() )
			{
				// ディレクトリではない

				continue;
			}

			// 再帰呼び出し

			FindFile( rfvcPathToDstFile, clffTarget.GetFilePath(), pszTargetFileName, bSearchSubDirectory );
		}
		while( clffTarget.FindNextFile() );

		bReturn = TRUE;
	}

	clffTarget.Close();

	return	bReturn;
}
