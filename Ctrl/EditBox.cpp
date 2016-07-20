#include "stdafx.h"
#include "EditBox.h"
#include "Utils/ArrayUtils.h"

HWND CEditBox::Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x, int y, int cx, int cy)
{
	Init(hWnd, ID);
	return CreateCtrl(_T("EDIT"), WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP, lpCaption, x, y, cx, cy, WS_EX_CLIENTEDGE);
}

HWND CEditBox::Create(HWND hWnd, DWORD dwCaption, UINT ID, int x, int y, int cx, int cy)
{
	TCHAR Caption[256];
	_stprintf(Caption, _T("%u"), dwCaption);
	return Create(hWnd, Caption, ID, x, y, cx, cy);
}

void CEditBox::GetText(LPTSTR pText, int len) const
{
	GetWindowText(GetCtrlHandle(), pText, len);
}

void CEditBox::GetText(YCString& pText) const
{
	TCHAR szText[256];
	GetWindowText(GetCtrlHandle(), szText, ArrayUtils::ArraySize(szText));
	pText = szText;
}

void CEditBox::GetText(LPDWORD pText, BOOL bHex) const
{
	TCHAR szText[256];
	GetWindowText(GetCtrlHandle(), szText, ArrayUtils::ArraySize(szText));
	*pText = (lstrcmp(szText, _T("")) == 0) ? 0 : (bHex == FALSE) ? _tstoi(szText) : _tcstol(szText, nullptr, 16);
}

void CEditBox::SetText(LPCTSTR pText)
{
	SetWindowText(GetCtrlHandle(), pText);
}

void CEditBox::SetLimit(int length)
{
	SendMessage(GetCtrlHandle(), EM_SETLIMITTEXT, static_cast<WPARAM>(length), 0);
}
