#pragma once

class CWinMain : public CWindowBase
{
protected:
	// Declaration for use in a member function
	WNDCLASSEX m_wc;

	// WinMain Arguments
	HINSTANCE m_hInst;
	HINSTANCE m_hPrevInst;
	LPTSTR m_lpsCmdLine;
	int m_nCmdShow;

public:
	CWinMain();
	~CWinMain();

	int WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpsCmdLine, int nCmdShow);
	ATOM InitApp();
	BOOL InitInstance();

	LRESULT WndProc(HWND hWnd, UINT Msg, WPARAM wp, LPARAM lp);

	void CreateMenu(LPARAM lp);
	void SetQuickMenu(HMENU hMenu);
	void SetQuickMenuItem(int nID);
};
