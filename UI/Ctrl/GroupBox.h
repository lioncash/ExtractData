#pragma once

#include "UI/Ctrl/CtrlBase.h"

class CGroupBox : public CCtrlBase
{
public:
	HWND Create(HWND window, LPCTSTR caption, UINT id, int x, int y, int cx, int cy) override;
};
