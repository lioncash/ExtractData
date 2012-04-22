
#include	"stdafx.h"
#include	"YCIni.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

YCIni::YCIni(
	LPCTSTR				pszPathToIni					// INIファイルパス
	)
{
	// 実行ファイルパスの取得

	TCHAR				szPathToExecuteFolder[MAX_PATH];

	::GetModuleFileName( NULL, szPathToExecuteFolder, _countof( szPathToExecuteFolder ) );
	::PathRemoveFileSpec( szPathToExecuteFolder );

	// INIファイルパスの取得

	m_clsPathToIni.Format( _T("%s\\%s"), szPathToExecuteFolder, pszPathToIni );

//	m_clsPathToIni = pszPathToIni;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	セクション名を設定

void	YCIni::SetSection(
	LPCTSTR				pszSection						// セクション名
	)
{
	m_clsSection = pszSection;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	セクション名を設定

void	YCIni::SetSection(
	UINT				uID								// ストリングID
	)
{
	TCHAR				szSection[256];

	::LoadString( ::GetModuleHandle( NULL ), uID, szSection, _countof( szSection ) );

	SetSection( szSection );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	キー名を設定

void	YCIni::SetKey(
	LPCTSTR				pszKey							// キー名
	)
{
	m_clsKey = pszKey;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列を取得

void	YCIni::ReadStr(
	LPTSTR				pszDst,							// 文字列の格納先
	DWORD				dwDstSize,						// 格納バッファサイズ
	LPCTSTR				pszDefault						// デフォルト値
	)
{
	::GetPrivateProfileString( m_clsSection, m_clsKey, pszDefault, pszDst, dwDstSize, m_clsPathToIni );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列を取得

void	YCIni::ReadStr(
	YCString&			rfclsDst,						// 文字列の格納先
	const YCString&		rfclsDefault					// デフォルト値
	)
{
	TCHAR				szDst[1024];

	ReadStr( szDst, _countof( szDst ), rfclsDefault );

	rfclsDst = szDst;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	文字列を設定

void	YCIni::WriteStr(
	LPCTSTR				pszStr							// 設定値
	)
{
	::WritePrivateProfileString( m_clsSection, m_clsKey, pszStr, m_clsPathToIni );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	セクションを削除

BOOL	YCIni::DeleteSection(
	LPCTSTR				pszSection						// セクション名
	)
{
	if( pszSection == NULL )
	{
		pszSection = m_clsSection;
	}

	return	::WritePrivateProfileString( pszSection, NULL, NULL, m_clsPathToIni );
}
