
#include	"stdafx.h"
#include	"YCUtil.h"
/*
//////////////////////////////////////////////////////////////////////////////////////////
//	ファイルフォーマットを設定

YCString	YCUtil::SetFileFormat(
	const YCString&		rfcstFileName					// ファイル名
	)
{
	// 拡張子を取得

	LPCTSTR				pszFileExt = PathFindExtension( rfcstFileName );

	if( pszFileExt == NULL )
	{
		// 拡張子が存在しない

		return	_T("");
	}

	if( lstrlen( pszFileExt ) < 2 )
	{
		// 拡張子が"."しかない

		return	_T("");
	}

	// 拡張子を大文字に変換

	TCHAR				szFileFormat[256];

	lstrcpy( szFileFormat, &pszFileExt[1] );
	CharUpper( szFileFormat );

	return	szFileFormat;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	数値を指定した桁ごとにカンマで区切る

YCString	YCUtil::SetCommaFormat(
	DWORD				dwSize,							// 数値
	DWORD				dwDigit							// 区切る桁
	)
{
	YCString			cstSize;

	cstSize.Format( _T("%d"), dwSize );

	int					nLength = cstSize.length();

	int					comma_num = (nLength - 1) / dwDigit;
	int					comma_pos = nLength % dwDigit;

	if( comma_pos == 0 )
	{
		comma_pos = dwDigit;
	}
	if( comma_num == 0 )
	{
		comma_pos = 0;
	}

	for( int i = 0 ; i < comma_num ; i++ )
	{
		cstSize.insert( comma_pos + dwDigit * i + i, _T(",") );
	}

	return	cstSize;
}
*/
//////////////////////////////////////////////////////////////////////////////////////////
//	作成したい最下層のディレクトリまで作成

void	YCUtil::CreateDirectory(
	LPCTSTR				pszPathToFile,					// ファイルパス
	BOOL				bFileName						// ファイルパスにファイル名が含まれているか(TRUE：含まれている、FALSE：含まれていない)
	)
{
	std::vector<YCString>	vcDirPathList;
	LPCTSTR					pszFilePathBase = pszPathToFile;

	while( (pszPathToFile = PathFindNextComponent( pszPathToFile )) != NULL )
	{
		YCString				clsDirPath( pszFilePathBase, pszPathToFile - pszFilePathBase - 1 ); // 念のため-1して末尾に'\'を付けないようにする

		vcDirPathList.push_back( clsDirPath );
	}

	// ルートから順にディレクトリを作成

	size_t					uMax = vcDirPathList.size();

	if( bFileName )
	{
		// ファイル名のディレクトリを作成しないように-1する

		uMax--;
	}

	for( size_t uCnt = 0 ; uCnt < uMax ; uCnt++ )
	{
		::CreateDirectory( vcDirPathList[uCnt], NULL );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	スラッシュをバックスラッシュに置換

void	YCUtil::ReplaceSlashToBackslash(
	LPSTR				pszFileName						// ファイル名
	)
{
	while( *pszFileName != '\0' )
	{
		if( !::IsDBCSLeadByte( *pszFileName ) )
		{
			// 1バイト文字

			// スラッシュをバックスラッシュに置換

			if( *pszFileName == '/' )
			{
				*pszFileName = '\\';
			}
		}

		// 次の文字へ

		pszFileName = ::CharNextA( pszFileName );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	スラッシュをバックスラッシュに置換

void	YCUtil::ReplaceSlashToBackslash(
	LPWSTR				pwszFileName					// ファイル名
	)
{
	for( ; *pwszFileName != L'\0' ; pwszFileName++ )
	{
		if( *pwszFileName == L'/' )
		{
			// スラッシュ

			*pwszFileName = L'\\';
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	エンディアン変換(4byte)

DWORD	YCUtil::ConvEndian(
	DWORD				dwSrc
	)
{
	_asm
	{
		mov				eax, dwSrc
		bswap			eax
		mov				dwSrc, eax
	}

	return	dwSrc;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	エンディアン変換(4byte)

void	YCUtil::ConvEndian(
	LPDWORD				pdwDst
	)
{
	*pdwDst = ConvEndian( *pdwDst );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	エンディアン変換(2byte)

WORD	YCUtil::ConvEndian(
	WORD				wSrc
	)
{
	_asm
	{
		mov				ax, wSrc
		rol				ax, 8
		mov				wSrc, ax
	}

	return	wSrc;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	エンディアン変換(2byte)

void	YCUtil::ConvEndian(
	LPWORD				pwDst
	)
{
	*pwDst = ConvEndian( *pwDst );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	メモリの内容を比較
//
//	備考	標準関数memcmp()のワイルドカード対応版

BOOL	YCUtil::CompareMemory(
	const void*			pvData1,						// 比較データ1
	const void*			pvData2,						// 比較データ2
	DWORD				dwSize							// 比較サイズ
	)
{
	BYTE*				pbtData1 = (BYTE*)pvData1;
	BYTE*				pbtData2 = (BYTE*)pvData2;

	for( DWORD i = 0 ; i < dwSize ; i++ )
	{
		if( (pbtData1[i] != pbtData2[i]) && (pbtData2[i] != '*') )
		{
			// 不一致

			return	FALSE;
		}
	}

	// 一致

	return	TRUE;
}
