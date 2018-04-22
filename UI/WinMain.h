#pragma once

#include "UI/WindowBase.h"

class CWinMain : public CWindowBase
{
public:
	CWinMain();
	~CWinMain() override;

	int WinMain(HINSTANCE inst, HINSTANCE prev_inst, LPTSTR cmd_line, int cmd_show);
	ATOM InitApp();
	bool InitInstance();

	LRESULT WndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp) override;

	void CreateMenu(LPARAM lp);
	void SetQuickMenu(HMENU menu);
	void SetQuickMenuItem(int id);

protected:
	// Declaration for use in a member function
	WNDCLASSEX m_wc{};

	// WinMain Arguments
	HINSTANCE m_inst = nullptr;
	HINSTANCE m_prev_inst = nullptr;
	LPTSTR m_cmd_line = nullptr;
	int m_cmd_show = 0;
};
