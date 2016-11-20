#include "StdAfx.h"
#include "Common.h"
#include "LastDir.h"

/// Constructor
CLastDir::CLastDir()
{
	LoadIni();
}

/// Load ini file
void CLastDir::LoadIni()
{
	YCIni ini(SBL_STR_INI_EXTRACTDATA);

	ini.SetSection(_T("LastDir"));

	ini.SetKey(_T("LastReadFileDir"));
	ini.ReadStr(m_open, sizeof(m_open), _T(""));
	ini.SetKey(_T("LastSaveFolderDir"));
	ini.ReadStr(m_save, sizeof(m_save), _T(""));
}

/// Save ini file
void CLastDir::SaveIni()
{
	YCIni ini(SBL_STR_INI_EXTRACTDATA);

	ini.SetSection(_T("LastDir"));

	ini.SetKey(_T("LastReadFileDir"));
	ini.WriteStr(m_open);
	ini.SetKey(_T("LastSaveFolderDir"));
	ini.WriteStr(m_save);
}

/// Get the last opened folder
LPTSTR  CLastDir::GetOpen()
{
	return m_open;
}

/// Get the last folder where something was saved to it
LPTSTR  CLastDir::GetSave()
{
	return m_save;
}
