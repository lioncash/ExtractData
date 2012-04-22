#include "stdafx.h"
#include "GroupBox.h"

HWND CGroupBox::Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x, int y, int cx, int cy)
{
	Init(hWnd, ID);
	return CreateCtrl(_T("BUTTON"), BS_GROUPBOX, lpCaption, x, y, cx, cy);
}
