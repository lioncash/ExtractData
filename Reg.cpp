#include "StdAfx.h"
#include "Common.h"
#include "Reg.h"

bool CReg::GetValue(YCString& Value, LPCTSTR pKeyPath, LPCTSTR pKeyName)
{
	// Retrieve the handle of the key name
	LPTSTR pKeyPathNext = PathFindNextComponent(pKeyPath);
	YCString sKey(pKeyPath, pKeyPathNext - pKeyPath - 1);
	// Convert to a numeric handle name
	HKEY hKey;
	if (sKey == _T("HKEY_CLASSES_ROOT"))
		hKey = HKEY_CLASSES_ROOT;
	else if (sKey == _T("HKEY_CURRENT_USER"))
		hKey = HKEY_CURRENT_USER;
	else if (sKey == _T("HKEY_LOCAL_MACHINE"))
		hKey = HKEY_LOCAL_MACHINE;
	else if (sKey == _T("HKEY_USERS"))
		hKey = HKEY_USERS;
	else if (sKey == _T("HKEY_CURRENT_CONFIG"))
		hKey = HKEY_CURRENT_CONFIG;
	else
		return false;

	// Retrieve the name of the subkey
	LPCTSTR pKeyPathEnd = pKeyPath + lstrlen(pKeyPath);
	YCString sSubKey(pKeyPathNext, pKeyPathEnd - pKeyPathNext);

	HKEY hkResult;
	if (RegOpenKeyEx(hKey, sSubKey, 0, KEY_QUERY_VALUE, &hkResult) != 0)
		return false;

	BYTE data[MAX_PATH];
	DWORD cbData = sizeof(data);
	if (RegQueryValueEx(hkResult, pKeyName, nullptr, nullptr, data, &cbData) != 0)
		return false;

	Value = reinterpret_cast<LPTSTR>(data);

	RegCloseKey(hkResult);

	return true;
}
