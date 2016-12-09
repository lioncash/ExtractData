#include "StdAfx.h"
#include "UI/Ctrl/Label.h"

HWND CLabel::Create(HWND window, LPCTSTR caption, UINT id, int x, int y, int cx, int cy)
{
	Init(window, id);
	return CreateCtrl(_T("STATIC"), SS_NOTIFY, caption, x, y, cx, cy);
}
