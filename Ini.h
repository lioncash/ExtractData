#pragma once

#include "Common.h"

class CIni
{
private:
	YCString m_sIniPath;
	YCString m_sSection;
	YCString m_sKey;

public:
	CIni();
	CIni(LPCTSTR IniName);

	void SetSection(HINSTANCE hInst, UINT uID);
	void SetSection(LPCTSTR section) { m_sSection = section; }
	void SetKey(LPCTSTR key) { m_sKey = key; }

	template <typename _Ty>
	void GetDec(_Ty* dec, CONST _Ty def)
	{
		*dec = GetPrivateProfileInt(m_sSection, m_sKey, def, m_sIniPath);
	}

	template <typename _Ty>
	void GetDec(_Ty* dec)
	{
		*dec = GetPrivateProfileInt(m_sSection, m_sKey, *dec, m_sIniPath);
	}

	void GetHex(LPDWORD hex, LPCTSTR def);
	void GetStr(LPTSTR str, LPCTSTR def, CONST DWORD len);
	void GetStr(YCString& str, YCString def);

	void WriteBool(BOOL flag);
	void WriteDec(INT dec);
	void WriteHex(DWORD hex);
	void WriteStr(LPCTSTR str);

	BOOL DeleteSection();
};
