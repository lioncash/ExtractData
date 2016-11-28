#include "StdAfx.h"
#include "UI/Ctrl/CtrlBase.h"

CCtrlBase::CCtrlBase()
{
}

void CCtrlBase::Init(HWND hWnd, UINT ID)
{
	m_hWnd = hWnd;
	m_hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
	m_ID = ID;
	m_font.Create();
}

HWND CCtrlBase::GetHandle() const
{
	return m_hWnd;
}

HINSTANCE CCtrlBase::GetInst() const
{
	return m_hInst;
}

HWND CCtrlBase::GetCtrlHandle() const
{
	return m_hCtrl;
}

UINT CCtrlBase::GetID() const
{
	return m_ID;
}

HFONT CCtrlBase::GetFont() const
{
	return m_font.GetFont();
}

HWND CCtrlBase::CreateCtrl(LPCTSTR lpClassName, DWORD dwStyle, LPCTSTR lpCaption, int x, int y, int cx, int cy, DWORD dwExStyle)
{
	dwStyle |= WS_CHILD | WS_VISIBLE;
	m_hCtrl = ::CreateWindowEx(dwExStyle,
		lpClassName, lpCaption, dwStyle,
		x, y, cx, cy,
		GetHandle(), reinterpret_cast<HMENU>(m_ID), GetInst(), nullptr);

	SetFont();

	return m_hCtrl;
}

void CCtrlBase::SetFont()
{
	SendMessage(GetCtrlHandle(), WM_SETFONT, (WPARAM)GetFont(), MAKELPARAM(TRUE, 0));
}

void CCtrlBase::SetFocus()
{
	::SetFocus(GetCtrlHandle());
}

void CCtrlBase::SetWindowPos(int x, int y, int cx, int cy)
{
	MoveWindow(GetCtrlHandle(), x, y, cx, cy, TRUE);
};

void CCtrlBase::Enable(BOOL flag)
{
	EnableWindow(m_hCtrl, flag);
}
