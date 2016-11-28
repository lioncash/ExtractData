#pragma once

#include "UI/WindowBase.h"

class CWinMain : public CWindowBase
{
public:
	CWinMain();
	~CWinMain();

	int WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpsCmdLine, int nCmdShow);
	ATOM InitApp();
	bool InitInstance();

	LRESULT WndProc(HWND hWnd, UINT Msg, WPARAM wp, LPARAM lp) override;

	void CreateMenu(LPARAM lp);
	void SetQuickMenu(HMENU hMenu);
	void SetQuickMenuItem(int nID);

protected:
	// Declaration for use in a member function
	WNDCLASSEX m_wc{};

	// WinMain Arguments
	HINSTANCE m_hInst = nullptr;
	HINSTANCE m_hPrevInst = nullptr;
	LPTSTR m_lpsCmdLine = nullptr;
	int m_nCmdShow = 0;
};
