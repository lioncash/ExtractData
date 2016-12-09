#pragma once

#include "UI/Ctrl/CtrlBase.h"

class CLabel : public CCtrlBase
{
public:
	HWND Create(HWND window, LPCTSTR caption, UINT id, int x = 0, int y = 0, int cx = 0, int cy = 0) override;
};
