#pragma once

#include "CtrlBase.h"

class CEditBox : public CCtrlBase
{
public:
	HWND Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x = 0, int y = 0, int cx = 0, int cy = 0) override;
	HWND Create(HWND hWnd, DWORD dwCaption, UINT ID, int x = 0, int y = 0, int cx = 0, int cy = 0);
	void GetText(LPTSTR pText, int len);
	void GetText(YCString& pText);
	void GetText(LPDWORD pText, BOOL bHex);
	void SetText(LPCTSTR pText);
	void SetLimit(int length);
};
