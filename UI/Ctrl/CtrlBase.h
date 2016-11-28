#pragma once

#include "UI/Ctrl/Font.h"

class CCtrlBase
{
public:
	CCtrlBase();
	virtual ~CCtrlBase() = default;

	void Init(HWND hWnd, UINT ID);

	virtual HWND Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x = 0, int y = 0, int cx = 0, int cy = 0) = 0;

	HWND      GetHandle()     const { return m_hWnd; }
	HINSTANCE GetInst()       const { return m_hInst; }
	HWND      GetCtrlHandle() const { return m_hCtrl; }
	UINT      GetID()         const { return m_ID; }
	HFONT     GetFont()       const { return m_font.GetFont(); }

	void      SetFont()       { SendMessage(GetCtrlHandle(), WM_SETFONT, (WPARAM)GetFont(), MAKELPARAM(TRUE, 0)); }
	void      SetFocus()      { ::SetFocus(GetCtrlHandle()); }
	void      SetWindowPos(int x, int y, int cx, int cy) { MoveWindow(GetCtrlHandle(), x, y, cx, cy, TRUE); };

	void Enable(BOOL flag);

protected:
	HWND CreateCtrl(LPCTSTR lpClassName, DWORD dwStyle, LPCTSTR lpCaption, int x, int y, int cx, int cy, DWORD dwExStyle = 0);

private:
	HWND      m_hWnd = nullptr;
	HINSTANCE m_hInst = nullptr;
	HWND      m_hCtrl = nullptr;
	UINT      m_ID = 0;
	CFont     m_font;
};
