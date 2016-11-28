#include "StdAfx.h"
#include "UI/Ctrl/Button.h"

// Function that generates / creates a button
HWND CButton::Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x, int y, int cx, int cy)
{
	Init(hWnd, ID);
	return CreateCtrl(_T("BUTTON"), BS_PUSHBUTTON | WS_TABSTOP, lpCaption, x, y, cx, cy);
}

void CButton::SetDef()
{
	LONG_PTR style = GetWindowLongPtr(GetCtrlHandle(), GWL_STYLE);
	style |= BS_DEFPUSHBUTTON;
	SetWindowLongPtr(GetCtrlHandle(), GWL_STYLE, style);
}
