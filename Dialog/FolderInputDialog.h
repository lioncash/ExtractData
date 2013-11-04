#pragma once

#include "../WindowBase.h"

class CFolderInputDialog : public CWindowBase
{
private:
	LPTSTR m_pSaveDir;

public:
	INT_PTR DoModal(HWND hWnd, LPTSTR pSaveDir);
	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};
