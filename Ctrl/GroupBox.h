#pragma once

#include "CtrlBase.h"

class CGroupBox : public CCtrlBase
{
public:
	HWND Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x, int y, int cx, int cy);
};
