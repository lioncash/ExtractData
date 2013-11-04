#pragma	once

class CLastDir
{
public:

	CLastDir();

	void   LoadIni();
	void   SaveIni();

	LPTSTR GetOpen();
	LPTSTR GetSave();


private:

	TCHAR  m_szOpen[_MAX_DIR];
	TCHAR  m_szSave[_MAX_DIR];
};
