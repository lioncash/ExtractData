#pragma once

class CToolBar
{
public:
	CToolBar();
	virtual ~CToolBar() = default;

	void Init(HWND window);
	HWND Create(HWND window, LPCTBBUTTON toolbar_button, UINT bitmap_id, int icon_cx, int ctIcon);

	virtual void LoadIni() {}
	virtual void SaveIni() {}

	virtual void SetWindowPos();

	HWND GetHandle() const;
	HWND GetCtrlHandle() const;

private:
	HWND m_window = nullptr;
	HINSTANCE m_inst = nullptr;
	HWND m_tool_bar = nullptr;
};
