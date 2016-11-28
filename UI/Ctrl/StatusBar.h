#pragma once

#include "UI/MainListView.h"

class CStatusBar
{
public:
	CStatusBar();

	void Init(HWND hWnd, SOption& option, CMainListView& listview);
	HWND Create(HWND hWnd, SOption& option, CMainListView& listview);
	void SetCount();
	void SetWindowPos(int cx);

protected:
	HWND m_hWnd = nullptr;
	HWND m_hSBWnd = nullptr;
	SOption* m_pOption = nullptr;
	CMainListView* m_pListView = nullptr;
};
