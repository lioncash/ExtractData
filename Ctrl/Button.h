#pragma once

#include "CtrlBase.h"

class CButton : public CCtrlBase
{
public:
	HWND Create(HWND hWnd, LPCTSTR lpCaption, UINT uID, int x = 0, int y = 0, int cx = 0, int cy = 0) override;
	void SetDef();
};
