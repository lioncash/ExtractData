
#include "stdafx.h"
#include "YCIni.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

YCIni::YCIni(LPCTSTR pszPathToIni)
{
	// Gets the execution path of the file
	TCHAR szPathToExecuteFolder[MAX_PATH];
	::GetModuleFileName(NULL, szPathToExecuteFolder, _countof(szPathToExecuteFolder));
	::PathRemoveFileSpec(szPathToExecuteFolder);

	// Get INI file path
	m_clsPathToIni.Format(_T("%s\\%s"), szPathToExecuteFolder, pszPathToIni);

// m_clsPathToIni = pszPathToIni;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Set a section name

void YCIni::SetSection(LPCTSTR pszSection)
{
	m_clsSection = pszSection;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Set a section name
//
// Parameters:
//   - uID - String ID

void YCIni::SetSection(UINT uID)
{
	TCHAR szSection[256];

	::LoadString(::GetModuleHandle(NULL), uID, szSection, _countof(szSection));

	SetSection(szSection);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Set key name

void YCIni::SetKey(LPCTSTR pszKey)
{
	m_clsKey = pszKey;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets a string
//
// Parameters:
//   - pszDst     - Storage location for the string
//   - dwDstSize  - Buffer size
//   - pszDefault - Default value

void YCIni::ReadStr(LPTSTR pszDst, DWORD dwDstSize, LPCTSTR pszDefault)
{
	::GetPrivateProfileString(m_clsSection, m_clsKey, pszDefault, pszDst, dwDstSize, m_clsPathToIni);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets a string
//
// Parameters:
//   - rfclsDst     - Storage location for the string
//   - rfclsDefault - Default value

void YCIni::ReadStr(YCString& rfclsDst, const YCString& rfclsDefault)
{
	TCHAR szDst[1024];

	ReadStr(szDst, _countof( szDst ), rfclsDefault);

	rfclsDst = szDst;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Sets the string

void YCIni::WriteStr(LPCTSTR pszStr)
{
	::WritePrivateProfileString(m_clsSection, m_clsKey, pszStr, m_clsPathToIni);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Delete section

BOOL YCIni::DeleteSection(LPCTSTR pszSection)
{
	if (pszSection == NULL)
	{
		pszSection = m_clsSection;
	}

	return ::WritePrivateProfileString(pszSection, NULL, NULL, m_clsPathToIni);
}
