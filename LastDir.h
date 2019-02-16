#pragma once

class CLastDir
{
public:
	CLastDir();

	void LoadIni();
	void SaveIni();

	LPTSTR GetOpen();
	LPCTSTR GetOpen() const;

	LPTSTR GetSave();
	LPCTSTR GetSave() const;

private:
	TCHAR m_open[_MAX_DIR];
	TCHAR m_save[_MAX_DIR];
};
