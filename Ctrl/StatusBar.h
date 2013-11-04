#pragma once

#include "../MainListView.h"

class CStatusBar
{
protected:
	HWND m_hWnd;
	HWND m_hSBWnd;
	SOption* m_pOption;
	CMainListView* m_pListView;

public:
	CStatusBar();

	void Init(HWND hWnd, SOption& option, CMainListView& listview);
	HWND Create(HWND hWnd, SOption& option, CMainListView& listview);
	void SetCount();
	void SetWindowPos(int cx);
};
