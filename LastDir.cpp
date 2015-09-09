#include "stdafx.h"
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
	YCIni clIni(SBL_STR_INI_EXTRACTDATA);

	clIni.SetSection(_T("LastDir"));

	clIni.SetKey(_T("LastReadFileDir"));
	clIni.ReadStr(m_szOpen, sizeof(m_szOpen), _T(""));
	clIni.SetKey(_T("LastSaveFolderDir"));
	clIni.ReadStr(m_szSave, sizeof(m_szSave), _T(""));
}

/// Save ini file
void CLastDir::SaveIni()
{
	YCIni clIni(SBL_STR_INI_EXTRACTDATA);

	clIni.SetSection(_T("LastDir"));

	clIni.SetKey(_T("LastReadFileDir"));
	clIni.WriteStr(m_szOpen);
	clIni.SetKey(_T("LastSaveFolderDir"));
	clIni.WriteStr(m_szSave);
}

/// Get the last opened folder
LPTSTR  CLastDir::GetOpen()
{
	return m_szOpen;
}

/// Get the last folder where something was saved to it
LPTSTR  CLastDir::GetSave()
{
	return m_szSave;
}
