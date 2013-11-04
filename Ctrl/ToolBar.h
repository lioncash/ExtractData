#pragma once

#include "../ArcFile.h"

class CToolBar
{
private:
	HWND m_hWnd;
	HINSTANCE m_hInst;
	HWND m_hToolBar;

public:
	CToolBar();

	void Init(HWND hWnd);
	HWND Create(HWND hWnd, LPTBBUTTON tbButton, UINT BmpID, int icon_cx, int ctIcon);

	virtual void LoadIni() {};
	virtual void SaveIni() {};

	virtual void SetWindowPos() { SendMessage(m_hToolBar, WM_SIZE, 0, 0); }

	HWND GetHandle() { return (m_hWnd); }
	HWND GetCtrlHandle() { return (m_hToolBar); }
};
