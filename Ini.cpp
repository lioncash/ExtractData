#include "stdafx.h"
#include "res/ResExtractData.h"
#include "Common.h"
#include "Ini.h"

CIni::CIni()
{
	TCHAR modulePath[MAX_PATH];
	GetModuleFileName(NULL, modulePath, sizeof(modulePath));
	PathRemoveFileSpec(modulePath);

	TCHAR szIniPath[MAX_PATH];
	wsprintf(szIniPath, _T("%s\\ExtractData.ini"), modulePath);

	m_sIniPath = szIniPath;
}

CIni::CIni(LPCTSTR IniName)
{
	TCHAR modulePath[MAX_PATH];
	GetModuleFileName(NULL, modulePath, sizeof(modulePath));
	PathRemoveFileSpec(modulePath);

	TCHAR szIniPath[MAX_PATH];
	wsprintf(szIniPath, _T("%s\\%s.ini"), modulePath, IniName);

	m_sIniPath = szIniPath;
}

void CIni::SetSection(HINSTANCE hInst, UINT uID)
{
	TCHAR section[256];
	LoadString(hInst, uID, section, sizeof(section));
	SetSection(section);
}

void CIni::GetHex(LPDWORD hex, LPCTSTR def)
{
	TCHAR str[256];
	GetPrivateProfileString(m_sSection, m_sKey, def, str, sizeof(str), m_sIniPath);
	*hex = strtoul( str, NULL, 16 );
}

void CIni::GetStr(LPTSTR str, LPCTSTR def, DWORD len)
{
	GetPrivateProfileString(m_sSection, m_sKey, def, str, len, m_sIniPath);
}

void CIni::GetStr(YCString& str, YCString def)
{
	TCHAR szStr[1024];
	GetPrivateProfileString(m_sSection, m_sKey, def, szStr, 1024, m_sIniPath);
	str = szStr;
}

void CIni::WriteBool(BOOL flag)
{
	TCHAR str[256];
	wsprintf(str, _T("%d"), flag);
	WritePrivateProfileString(m_sSection, m_sKey, str, m_sIniPath);
}

void CIni::WriteDec(INT dec)
{
	TCHAR str[256];
	wsprintf(str, _T("%d"), dec);
	WritePrivateProfileString(m_sSection, m_sKey, str, m_sIniPath);
}

void CIni::WriteHex(DWORD hex)
{
	TCHAR str[256];
	wsprintf(str, _T("%06x"), hex);
	WritePrivateProfileString(m_sSection, m_sKey, str, m_sIniPath);
}

void CIni::WriteStr(LPCTSTR str)
{
	WritePrivateProfileString(m_sSection, m_sKey, str, m_sIniPath);
}

BOOL CIni::DeleteSection()
{
	return WritePrivateProfileString(m_sSection, NULL, NULL, m_sIniPath);
}
