#pragma once

#include "UI/WindowBase.h"

class CFolderInputDialog : public CWindowBase
{
public:
	INT_PTR DoModal(HWND window, LPTSTR save_dir);
	LRESULT WndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp) override;

private:
	LPTSTR m_save_dir = nullptr;
};
