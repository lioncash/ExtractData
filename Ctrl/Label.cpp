#include "stdafx.h"
#include "Label.h"

HWND CLabel::Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x, int y, int cx, int cy)
{
	Init(hWnd, ID);
	return CreateCtrl(_T("STATIC"), SS_NOTIFY, lpCaption, x, y, cx, cy);
}
