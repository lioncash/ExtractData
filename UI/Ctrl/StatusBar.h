#pragma once

#include "UI/MainListView.h"

class CStatusBar
{
public:
	CStatusBar();

	void Init(HWND window, SOption& option, CMainListView& listview);
	HWND Create(HWND window, SOption& option, CMainListView& listview);
	void SetCount();
	void SetWindowPos(int cx);

protected:
	HWND m_window = nullptr;
	HWND m_status_bar_window = nullptr;
	SOption* m_option = nullptr;
	CMainListView* m_list_view = nullptr;
};
