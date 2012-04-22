
#include	"stdafx.h"
#include	"YCIni.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	Constructor

YCIni::YCIni(
	LPCTSTR				pszPathToIni					// INI File path
	)
{
	// Gets the execution path of the file

	TCHAR				szPathToExecuteFolder[MAX_PATH];

	::GetModuleFileName( NULL, szPathToExecuteFolder, _countof( szPathToExecuteFolder ) );
	::PathRemoveFileSpec( szPathToExecuteFolder );

	// Get INI file path

	m_clsPathToIni.Format( _T("%s\\%s"), szPathToExecuteFolder, pszPathToIni );

//	m_clsPathToIni = pszPathToIni;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Set a section name

void	YCIni::SetSection(
	LPCTSTR				pszSection						// Section name
	)
{
	m_clsSection = pszSection;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Set a section name

void	YCIni::SetSection(
	UINT				uID								// String ID
	)
{
	TCHAR				szSection[256];

	::LoadString( ::GetModuleHandle( NULL ), uID, szSection, _countof( szSection ) );

	SetSection( szSection );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Set key name

void	YCIni::SetKey(
	LPCTSTR				pszKey							// Key name
	)
{
	m_clsKey = pszKey;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Gets a string

void	YCIni::ReadStr(
	LPTSTR				pszDst,							// Storage location of string
	DWORD				dwDstSize,						// Store buffer size
	LPCTSTR				pszDefault						// Default value
	)
{
	::GetPrivateProfileString( m_clsSection, m_clsKey, pszDefault, pszDst, dwDstSize, m_clsPathToIni );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Gets a string

void	YCIni::ReadStr(
	YCString&			rfclsDst,						// Storage location of the string
	const YCString&		rfclsDefault					// Default value
	)
{
	TCHAR				szDst[1024];

	ReadStr( szDst, _countof( szDst ), rfclsDefault );

	rfclsDst = szDst;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Sets the string

void	YCIni::WriteStr(
	LPCTSTR				pszStr							// Setting
	)
{
	::WritePrivateProfileString( m_clsSection, m_clsKey, pszStr, m_clsPathToIni );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Delete section

BOOL	YCIni::DeleteSection(
	LPCTSTR				pszSection						// Section name
	)
{
	if( pszSection == NULL )
	{
		pszSection = m_clsSection;
	}

	return	::WritePrivateProfileString( pszSection, NULL, NULL, m_clsPathToIni );
}
