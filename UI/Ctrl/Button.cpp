#include "StdAfx.h"
#include "UI/Ctrl/Button.h"

// Function that generates / creates a button
HWND CButton::Create(HWND window, LPCTSTR caption, UINT id, int x, int y, int cx, int cy)
{
	Init(window, id);
	return CreateCtrl(_T("BUTTON"), BS_PUSHBUTTON | WS_TABSTOP, caption, x, y, cx, cy);
}

void CButton::SetDef()
{
	LONG_PTR style = GetWindowLongPtr(GetCtrlHandle(), GWL_STYLE);
	style |= BS_DEFPUSHBUTTON;
	SetWindowLongPtr(GetCtrlHandle(), GWL_STYLE, style);
}
