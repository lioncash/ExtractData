#pragma once

#include "Font.h"

class CCtrlBase {
private:
	HWND		m_hWnd;
	HINSTANCE	m_hInst;
	HWND		m_hCtrl;
	UINT		m_ID;
	CFont		m_font;

protected:
	HWND CreateCtrl(LPCTSTR lpClassName, DWORD dwStyle, LPCTSTR lpCaption, int x, int y, int cx, int cy, DWORD dwExStyle = 0);

public:
	CCtrlBase();
	void Init(HWND hWnd, UINT ID);

	virtual HWND Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x = 0, int y = 0, int cx = 0, int cy = 0) = 0;

	HWND		GetHandle()		{ return (m_hWnd); }
	HINSTANCE	GetInst()		{ return (m_hInst); }
	HWND		GetCtrlHandle()	{ return (m_hCtrl); }
	UINT		GetID()			{ return (m_ID); }
	HFONT		GetFont()		{ return (m_font.GetFont()); }

	void	SetFont()		{ SendMessage(GetCtrlHandle(), WM_SETFONT, (WPARAM)GetFont(), MAKELPARAM(TRUE, 0)); }
	void	SetFocus()		{ ::SetFocus(GetCtrlHandle()); }
	void	SetWindowPos(int x, int y, int cx, int cy) { MoveWindow(GetCtrlHandle(), x, y, cx, cy, TRUE); };

	void Enable(BOOL flag);
};
