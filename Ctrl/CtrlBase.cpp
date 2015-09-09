#include "stdafx.h"
#include "CtrlBase.h"

CCtrlBase::CCtrlBase()
{
	m_hWnd = nullptr;
	m_hInst = nullptr;
	m_hCtrl = nullptr;
}

void CCtrlBase::Init(HWND hWnd, UINT ID)
{
	m_hWnd = hWnd;
	m_hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
	m_ID = ID;
	m_font.Create();
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

void CCtrlBase::Enable(BOOL flag)
{
	EnableWindow(m_hCtrl, flag);
}
