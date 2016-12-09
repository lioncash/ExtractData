#pragma once

#include "UI/Ctrl/CtrlBase.h"

class CEditBox : public CCtrlBase
{
public:
	HWND Create(HWND window, LPCTSTR caption, UINT id, int x = 0, int y = 0, int cx = 0, int cy = 0) override;
	HWND Create(HWND window, DWORD caption_number, UINT id, int x = 0, int y = 0, int cx = 0, int cy = 0);
	void GetText(LPTSTR text, int length) const;
	void GetText(YCString& text) const;
	void GetText(LPDWORD text, BOOL hex) const;
	void SetText(LPCTSTR text);
	void SetLimit(int length);
};
