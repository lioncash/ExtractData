#pragma once

#include "CtrlBase.h"

class CEditBox : public CCtrlBase
{
public:
	HWND Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x = 0, int y = 0, int cx = 0, int cy = 0) override;
	HWND Create(HWND hWnd, DWORD dwCaption, UINT ID, int x = 0, int y = 0, int cx = 0, int cy = 0);
	void GetText(LPTSTR pText, int len) const;
	void GetText(YCString& pText) const;
	void GetText(LPDWORD pText, BOOL bHex) const;
	void SetText(LPCTSTR pText);
	void SetLimit(int length);
};
