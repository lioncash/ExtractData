#include "StdAfx.h"
#include "UI/Ctrl/CtrlBase.h"

CCtrlBase::CCtrlBase()
{
}

void CCtrlBase::Init(HWND window, UINT id)
{
	m_window = window;
	m_inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(window, GWLP_HINSTANCE));
	m_id = id;
	m_font.Create();
}

HWND CCtrlBase::GetHandle() const
{
	return m_window;
}

HINSTANCE CCtrlBase::GetInst() const
{
	return m_inst;
}

HWND CCtrlBase::GetCtrlHandle() const
{
	return m_ctrl;
}

UINT CCtrlBase::GetID() const
{
	return m_id;
}

HFONT CCtrlBase::GetFont() const
{
	return m_font.GetFont();
}

HWND CCtrlBase::CreateCtrl(LPCTSTR class_name, DWORD style, LPCTSTR caption, int x, int y, int cx, int cy, DWORD ex_style)
{
	style |= WS_CHILD | WS_VISIBLE;
	m_ctrl = ::CreateWindowEx(ex_style,
		class_name, caption, style,
		x, y, cx, cy,
		GetHandle(), reinterpret_cast<HMENU>(m_id), GetInst(), nullptr);

	SetFont();

	return m_ctrl;
}

void CCtrlBase::SetFont()
{
	SendMessage(GetCtrlHandle(), WM_SETFONT, reinterpret_cast<WPARAM>(GetFont()), MAKELPARAM(TRUE, 0));
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
	EnableWindow(m_ctrl, flag);
}
