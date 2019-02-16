#pragma once

//----------------------------------------------------------------------------------------
//-- INI Class ---------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class YCIni
{
public:
	explicit YCIni(LPCTSTR ini_path);
  ~YCIni();

	void SetSection(u32 id);
	void SetSection(LPCTSTR section);

	void SetKey(LPCTSTR key);

	template<typename TYPE> TYPE ReadDec(const TYPE& default_value);
	template<typename TYPE> void ReadDec(TYPE* dst, const TYPE& default_value);
	template<typename TYPE> void ReadDec(TYPE* dst);

	template<typename TYPE> TYPE ReadHex(LPCTSTR default_string);
	template<typename TYPE> void ReadHex(TYPE* dst, LPCTSTR default_string);

	void ReadStr(LPTSTR dst, DWORD dst_size, LPCTSTR default_string);
	void ReadStr(YCString& dst, const YCString& default_string);

	template<typename TYPE> void WriteDec(const TYPE& dec);

	template<typename TYPE> void WriteHex(const TYPE& hex, DWORD number = 1);

	void WriteStr(LPCTSTR str);

	bool DeleteSection(LPCTSTR section = nullptr);

private:
	YCString m_ini_path;
	YCString m_section;
	YCString m_key;
};

/// Get a numeric value
///
/// @param default_value Default value
///
template<typename TYPE>
TYPE YCIni::ReadDec(const TYPE& default_value)
{
	return ::GetPrivateProfileInt(m_section, m_key, default_value, m_ini_path);
}

/// Get a numeric value
///
/// @param dst           Storage location of a number
/// @param default_value Default value
///
template<typename TYPE>
void YCIni::ReadDec(TYPE* dst, const TYPE& default_value)
{
	*dst = ReadDec(default_value);
}

/// Get a numeric value
///
/// @param dst Storage location of a numeric value (default values are stored in the destination)
///
template<typename TYPE>
void YCIni::ReadDec(TYPE* dst)
{
	*dst = ReadDec(*dst);
}

/// Gets a hexadecimal (16) number
///
/// @param default_value Default value
///
template<typename TYPE>
TYPE YCIni::ReadHex(LPCTSTR default_value)
{
	TCHAR work[256];

	::GetPrivateProfileString(m_section, m_key, default_value, work, sizeof(work), m_ini_path);

	return strtoul(work, nullptr, 16);
}

/// Gets a hexadecimal (16) number
///
/// @param dst           Location of the hexadecimal buffer
/// @param default_value Default value
///
template<typename TYPE>
void YCIni::ReadHex(TYPE* dst, LPCTSTR default_value)
{
	*dst = ReadHex(default_value);
}

/// Set the values
///
/// @param decimal Setting
///
template<typename TYPE>
void YCIni::WriteDec(const TYPE& decimal)
{
	TCHAR work[256];

	_stprintf(work, _T("%d"), decimal);

	WriteStr(work);
}

/// Writes a hexadecimal (16) number
///
/// @param hex    Setting
/// @param number Number of digits
///
template<typename TYPE>
void YCIni::WriteHex(const TYPE& hex, DWORD number)
{
	TCHAR work[256];

	_stprintf(work, _T("%0*X"), number, hex);

	WriteStr(work);
}
