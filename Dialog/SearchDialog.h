#pragma once

#include "../WindowBase.h"

class CSearchDialog : public CWindowBase
{
private:
	LPCTSTR m_pclArcPath;

public:
	int DoModal(HWND hWnd, LPCTSTR pclArcPath);
	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};
