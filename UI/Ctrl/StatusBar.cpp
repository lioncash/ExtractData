#include "StdAfx.h"
#include "UI/Ctrl/Statusbar.h"

#include "Common.h"
#include "UI/MainListView.h"

constexpr u32 SB_SIZE = 2;

CStatusBar::CStatusBar()
{
}

void CStatusBar::Init(HWND window, SOption& option, CMainListView& listview)
{
	m_window = window;
	m_option = &option;
	m_list_view = &listview;
}

// Function that creates a status bar
HWND CStatusBar::Create(HWND window, SOption& option, CMainListView& listview)
{
	Init(window, option, listview);

	HWND status_bar_window = CreateStatusWindow(
		WS_CHILD | WS_VISIBLE | CCS_BOTTOM,
		nullptr,
		m_window,
		ID_STATUS);
	m_status_bar_window = status_bar_window;

	int sb_size[SB_SIZE] = {};
	SendMessage(status_bar_window, SB_SETPARTS, SB_SIZE, reinterpret_cast<LPARAM>(sb_size));

	SetCount();

	return status_bar_window;
}

// Function to update the status bar: Displays the number of files
void CStatusBar::SetCount()
{
	TCHAR file_count[256];
	_stprintf(file_count, _T("Files %d"), m_list_view->GetCount());
	SendMessage(m_status_bar_window, SB_SETTEXT, 0 | 0, reinterpret_cast<WPARAM>(file_count));
}

void CStatusBar::SetWindowPos(int cx)
{
	int sb_size[SB_SIZE];
	sb_size[1] = cx;
	sb_size[0] = cx / 2;
	SendMessage(m_status_bar_window, SB_SETPARTS, SB_SIZE, reinterpret_cast<LPARAM>(sb_size));
	SendMessage(m_status_bar_window, WM_SIZE, 0, 0); 
}
