#include "stdafx.h"
#include "CheckBox.h"

HWND CCheckBox::Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x, int y, int cx, int cy)
{
	Init(hWnd, ID);
	return CreateCtrl(_T("BUTTON"), BS_AUTOCHECKBOX | WS_TABSTOP, lpCaption, x, y, cx, cy);
}
