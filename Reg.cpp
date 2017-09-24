#include "StdAfx.h"
#include "Common.h"
#include "Reg.h"

bool CReg::GetValue(YCString& value, LPCTSTR key_path, LPCTSTR key_name)
{
	// Retrieve the handle of the key name
	const LPTSTR next_key_path_component = PathFindNextComponent(key_path);
	const YCString full_key(key_path, next_key_path_component - key_path - 1);

	// Convert to a numeric handle name
	HKEY key;
	if (full_key == _T("HKEY_CLASSES_ROOT"))
		key = HKEY_CLASSES_ROOT;
	else if (full_key == _T("HKEY_CURRENT_USER"))
		key = HKEY_CURRENT_USER;
	else if (full_key == _T("HKEY_LOCAL_MACHINE"))
		key = HKEY_LOCAL_MACHINE;
	else if (full_key == _T("HKEY_USERS"))
		key = HKEY_USERS;
	else if (full_key == _T("HKEY_CURRENT_CONFIG"))
		key = HKEY_CURRENT_CONFIG;
	else
		return false;

	// Retrieve the name of the subkey
	const LPCTSTR key_path_end = key_path + lstrlen(key_path);
	const YCString sub_key(next_key_path_component, key_path_end - next_key_path_component);

	HKEY result;
	if (RegOpenKeyEx(key, sub_key, 0, KEY_QUERY_VALUE, &result) != 0)
		return false;

	u8 data[MAX_PATH];
	DWORD data_size = sizeof(data);
	if (RegQueryValueEx(result, key_name, nullptr, nullptr, data, &data_size) != 0)
		return false;

	value = reinterpret_cast<LPTSTR>(data);

	RegCloseKey(result);

	return true;
}
