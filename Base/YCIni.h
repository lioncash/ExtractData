#pragma once

//----------------------------------------------------------------------------------------
//-- INI Class ---------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class YCIni
{
public:
	YCIni(LPCTSTR pszIniName);

	void SetSection(UINT uID);
	void SetSection(LPCTSTR pszSection);

	void SetKey(LPCTSTR pszKey);

	template<typename TYPE> TYPE ReadDec(const TYPE& rftDefault);
	template<typename TYPE> void ReadDec(TYPE* ptDst, const TYPE& rftDefault);
	template<typename TYPE> void ReadDec(TYPE* ptDst);

	template<typename TYPE> TYPE ReadHex(LPCTSTR pszDef);
	template<typename TYPE> void ReadHex(TYPE* ptDst, LPCTSTR pszDefault);

	void ReadStr(LPTSTR pszDst, DWORD dwDstSize, LPCTSTR pszDefault);
	void ReadStr(YCString& rfclsDst, const YCString& rfclsDefault);

	template<typename TYPE> void WriteDec(const TYPE& rftDec);

	template<typename TYPE> void WriteHex(const TYPE& rftHex, DWORD dwNumber = 1);

	void WriteStr(LPCTSTR pszStr);

	BOOL DeleteSection(LPCTSTR pszSection = NULL);

private:
	YCString m_clsPathToIni;
	YCString m_clsSection;
	YCString m_clsKey;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Get a numeric value
//
// Parameters:
//   - rftDefault - Default value

template<typename TYPE>
TYPE YCIni::ReadDec(const TYPE& rftDefault)
{
	return ::GetPrivateProfileInt(m_clsSection, m_clsKey, rftDefault, m_clsPathToIni);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get a numeric value
//
// Parameters:
//   - ptDst      - Storage location of a number
//   - rftDefault - Default value

template<typename TYPE>
void YCIni::ReadDec(TYPE* ptDst, const TYPE& rftDefault)
{
	*ptDst = ReadDec(rftDefault);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Get a numeric value
//
// Parameters:
//   - ptDst - Storage location of a numeric value (default values are stored in the destination)

template<typename TYPE>
void YCIni::ReadDec(TYPE* ptDst)
{
	*ptDst = ReadDec(*ptDst);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets a hexadecimal (16) number
//
// Parameters:
//   - pszDefault - Default value

template<typename TYPE>
TYPE YCIni::ReadHex(LPCTSTR pszDefault)
{
	TCHAR szWork[256];

	::GetPrivateProfileString(m_clsSection, m_clsKey, pszDefault, szWork, sizeof(szWork), m_clsPathToIni);

	return strtoul(szWork, NULL, 16);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets a hexadecimal (16) number
//
// Parameters:
//   - ptDst      - Location of the hexadecimal buffer
//   - pszDefault - Default value

template<typename TYPE>
void YCIni::ReadHex(TYPE* ptDst, LPCTSTR pszDefault)
{
	*ptDst = ReadHex(pszDefault);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Set the values
//
// Parameters:
//   - rftDec - Setting

template<typename TYPE>
void YCIni::WriteDec(const TYPE& rftDec)
{
	TCHAR szWork[256];

	_stprintf(szWork, _T("%d"), rftDec);

	WriteStr(szWork);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Writes a hexadecimal (16) number
//
// Parameters:
//   - rftHex - Setting
//   - dwNumber - Number of digits

template<typename TYPE>
void YCIni::WriteHex(const TYPE& rftHex, DWORD dwNumber)
{
	TCHAR szWork[256];

	_stprintf(szWork, _T("%0*X"), dwNumber, rftHex);

	WriteStr(szWork);
}
