#pragma once

class CToolBar
{
public:
	CToolBar();
	virtual ~CToolBar() = default;

	void Init(HWND hWnd);
	HWND Create(HWND hWnd, LPTBBUTTON tbButton, UINT BmpID, int icon_cx, int ctIcon);

	virtual void LoadIni() {};
	virtual void SaveIni() {};

	virtual void SetWindowPos() { SendMessage(m_hToolBar, WM_SIZE, 0, 0); }

	HWND GetHandle()     const { return m_hWnd; }
	HWND GetCtrlHandle() const { return m_hToolBar; }

private:
	HWND m_hWnd = nullptr;
	HINSTANCE m_hInst = nullptr;
	HWND m_hToolBar = nullptr;
};
