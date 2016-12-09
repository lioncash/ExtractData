#include "StdAfx.h"
#include "UI/Ctrl/CheckBox.h"

HWND CCheckBox::Create(HWND window, LPCTSTR caption, UINT id, int x, int y, int cx, int cy)
{
	Init(window, id);
	return CreateCtrl(_T("BUTTON"), BS_AUTOCHECKBOX | WS_TABSTOP, caption, x, y, cx, cy);
}

void CCheckBox::SetCheck(BOOL flag)
{
	Button_SetCheck(GetCtrlHandle(), flag);
}

BOOL CCheckBox::GetCheck() const
{
	return Button_GetCheck(GetCtrlHandle());
}
