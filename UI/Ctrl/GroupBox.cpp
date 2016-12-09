#include "StdAfx.h"
#include "UI/Ctrl/GroupBox.h"

HWND CGroupBox::Create(HWND window, LPCTSTR caption, UINT id, int x, int y, int cx, int cy)
{
	Init(window, id);
	return CreateCtrl(_T("BUTTON"), BS_GROUPBOX, caption, x, y, cx, cy);
}
