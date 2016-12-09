#include "StdAfx.h"
#include "UI/Ctrl/EditBox.h"

#include "Utils/ArrayUtils.h"

HWND CEditBox::Create(HWND window, LPCTSTR caption, UINT id, int x, int y, int cx, int cy)
{
	Init(window, id);
	return CreateCtrl(_T("EDIT"), WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP, caption, x, y, cx, cy, WS_EX_CLIENTEDGE);
}

HWND CEditBox::Create(HWND window, DWORD caption_number, UINT id, int x, int y, int cx, int cy)
{
	TCHAR caption[256];
	_stprintf(caption, _T("%u"), caption_number);
	return Create(window, caption, id, x, y, cx, cy);
}

void CEditBox::GetText(LPTSTR text, int length) const
{
	GetWindowText(GetCtrlHandle(), text, length);
}

void CEditBox::GetText(YCString& text) const
{
	TCHAR window_text[256];
	GetWindowText(GetCtrlHandle(), window_text, ArrayUtils::ArraySize(window_text));
	text = window_text;
}

void CEditBox::GetText(LPDWORD text, BOOL hex) const
{
	TCHAR window_text[256];
	GetWindowText(GetCtrlHandle(), window_text, ArrayUtils::ArraySize(window_text));
	*text = (lstrcmp(window_text, _T("")) == 0) ? 0 : (hex == FALSE) ? _tstoi(window_text) : _tcstol(window_text, nullptr, 16);
}

void CEditBox::SetText(LPCTSTR text)
{
	SetWindowText(GetCtrlHandle(), text);
}

void CEditBox::SetLimit(int length)
{
	SendMessage(GetCtrlHandle(), EM_SETLIMITTEXT, static_cast<WPARAM>(length), 0);
}
