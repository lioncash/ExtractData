#include "StdAfx.h"
#include "UI/Ctrl/CheckBox.h"

HWND CCheckBox::Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x, int y, int cx, int cy)
{
	Init(hWnd, ID);
	return CreateCtrl(_T("BUTTON"), BS_AUTOCHECKBOX | WS_TABSTOP, lpCaption, x, y, cx, cy);
}

void CCheckBox::SetCheck(BOOL flag)
{
	Button_SetCheck(GetCtrlHandle(), flag);
}

BOOL CCheckBox::GetCheck() const
{
	return Button_GetCheck(GetCtrlHandle());
}
