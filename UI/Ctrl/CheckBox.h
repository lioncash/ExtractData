#pragma once

#include "UI/Ctrl/CtrlBase.h"

class CCheckBox : public CCtrlBase
{
public:
	HWND Create(HWND hWnd, LPCTSTR lpCaption, UINT uID, int x = 0, int y = 0, int cx = 0, int cy = 0) override;

	void SetCheck(BOOL flag)
	{
		Button_SetCheck(GetCtrlHandle(), flag);
	}

	BOOL GetCheck() const
	{
		return Button_GetCheck(GetCtrlHandle());
	}
};
