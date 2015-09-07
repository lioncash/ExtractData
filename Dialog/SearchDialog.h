#pragma once

#include "../WindowBase.h"

class CSearchDialog : public CWindowBase
{
public:
	int DoModal(HWND hWnd, LPCTSTR pclArcPath);
	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) override;

private:
	LPCTSTR m_pclArcPath;
};
