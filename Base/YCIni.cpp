#include "StdAfx.h"
#include "YCIni.h"
#include "Utils/ArrayUtils.h"

/// Constructor
YCIni::YCIni(LPCTSTR ini_path)
{
	// Gets the execution path of the file
	TCHAR exe_folder_path[MAX_PATH];
	::GetModuleFileName(nullptr, exe_folder_path, ArrayUtils::ArraySize(exe_folder_path));
	::PathRemoveFileSpec(exe_folder_path);

	// Get INI file path
	m_ini_path.Format(_T("%s\\%s"), exe_folder_path, ini_path);

// m_ini_path = ini_path;
}

/// Set a section name
void YCIni::SetSection(LPCTSTR section)
{
	m_section = section;
}

/// Set a section name
///
/// @param id String ID
///
void YCIni::SetSection(u32 id)
{
	TCHAR buffer[256];

	::LoadString(::GetModuleHandle(nullptr), id, buffer, ArrayUtils::ArraySize(buffer));

	SetSection(buffer);
}

/// Set key name
void YCIni::SetKey(LPCTSTR key)
{
	m_key = key;
}

/// Gets a string
///
/// @param dst            Storage location for the string
/// @param dst_size       Buffer size
/// @param default_string Default value
///
void YCIni::ReadStr(LPTSTR dst, DWORD dst_size, LPCTSTR default_string)
{
	::GetPrivateProfileString(m_section, m_key, default_string, dst, dst_size, m_ini_path);
}

/// Gets a string
///
/// @param dst            Storage location for the string
/// @param default_string Default value
///
void YCIni::ReadStr(YCString& dst, const YCString& default_string)
{
	TCHAR buffer[1024];

	ReadStr(buffer, ArrayUtils::ArraySize(buffer), default_string);

	dst = buffer;
}

/// Sets the string
void YCIni::WriteStr(LPCTSTR str)
{
	::WritePrivateProfileString(m_section, m_key, str, m_ini_path);
}

/// Delete section
bool YCIni::DeleteSection(LPCTSTR section)
{
	if (section == nullptr)
		section = m_section;

	return ::WritePrivateProfileString(section, nullptr, nullptr, m_ini_path) != 0;
}
