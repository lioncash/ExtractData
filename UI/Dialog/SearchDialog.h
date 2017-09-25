#pragma once

#include "UI/WindowBase.h"

class CSearchDialog : public CWindowBase
{
public:
	INT_PTR DoModal(HWND window, LPCTSTR archive_path);
	LRESULT WndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp) override;

private:
	LPCTSTR m_archive_path = nullptr;
};
