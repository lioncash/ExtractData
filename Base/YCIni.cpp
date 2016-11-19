#include "StdAfx.h"
#include "YCIni.h"
#include "Utils/ArrayUtils.h"

/// Constructor
YCIni::YCIni(LPCTSTR pszPathToIni)
{
	// Gets the execution path of the file
	TCHAR szPathToExecuteFolder[MAX_PATH];
	::GetModuleFileName(nullptr, szPathToExecuteFolder, ArrayUtils::ArraySize(szPathToExecuteFolder));
	::PathRemoveFileSpec(szPathToExecuteFolder);

	// Get INI file path
	m_clsPathToIni.Format(_T("%s\\%s"), szPathToExecuteFolder, pszPathToIni);

// m_clsPathToIni = pszPathToIni;
}

/// Set a section name
void YCIni::SetSection(LPCTSTR pszSection)
{
	m_clsSection = pszSection;
}

/// Set a section name
///
/// @param uID String ID
///
void YCIni::SetSection(UINT uID)
{
	TCHAR szSection[256];

	::LoadString(::GetModuleHandle(nullptr), uID, szSection, ArrayUtils::ArraySize(szSection));

	SetSection(szSection);
}

/// Set key name
void YCIni::SetKey(LPCTSTR pszKey)
{
	m_clsKey = pszKey;
}

/// Gets a string
///
/// @param pszDst     Storage location for the string
/// @param dwDstSize  Buffer size
/// @param pszDefault Default value
///
void YCIni::ReadStr(LPTSTR pszDst, DWORD dwDstSize, LPCTSTR pszDefault)
{
	::GetPrivateProfileString(m_clsSection, m_clsKey, pszDefault, pszDst, dwDstSize, m_clsPathToIni);
}

/// Gets a string
///
/// @param rfclsDst     Storage location for the string
/// @param rfclsDefault Default value
///
void YCIni::ReadStr(YCString& rfclsDst, const YCString& rfclsDefault)
{
	TCHAR szDst[1024];

	ReadStr(szDst, ArrayUtils::ArraySize(szDst), rfclsDefault);

	rfclsDst = szDst;
}

/// Sets the string
void YCIni::WriteStr(LPCTSTR pszStr)
{
	::WritePrivateProfileString(m_clsSection, m_clsKey, pszStr, m_clsPathToIni);
}

/// Delete section
bool YCIni::DeleteSection(LPCTSTR pszSection)
{
	if (pszSection == nullptr)
		pszSection = m_clsSection;

	return ::WritePrivateProfileString(pszSection, nullptr, nullptr, m_clsPathToIni) != 0;
}
