#pragma once

class CLastDir
{
public:
	CLastDir();

	void LoadIni();
	void SaveIni();

	LPTSTR GetOpen();
	LPTSTR GetSave();

private:
	TCHAR m_open[_MAX_DIR];
	TCHAR m_save[_MAX_DIR];
};
