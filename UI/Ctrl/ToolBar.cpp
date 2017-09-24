#include "StdAfx.h"
#include "UI/Ctrl/Toolbar.h"

#include "Common.h"

CToolBar::CToolBar()
{
}

void CToolBar::Init(HWND window)
{
	m_window = window;
	m_inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(window, GWLP_HINSTANCE));
	LoadIni();
}

// Function that creates the toolbar
HWND CToolBar::Create(HWND window, LPCTBBUTTON tool_bar_button, UINT bitmap_id, int icon_cx, int ctIcon)
{
	Init(window);

	m_tool_bar = CreateWindowEx(
		0,                                   // Extended window style
		TOOLBARCLASSNAME,                    // Class name
		nullptr,                             // Window name
		WS_CHILD | CCS_NORESIZE | TBSTYLE_WRAPABLE | TBSTYLE_TOOLTIPS, // Window style
		0,                                   // Initial horizontal position
		0,                                   // Initial vertical position
		0,                                   // Window width (device units)
		0,                                   // Window height (device units)
		window,                              // Window parent
		reinterpret_cast<HMENU>(ID_TOOLBAR), // Menu handle
		m_inst,                              // Module instance
		nullptr                              // WM_CREATE data
	);

	// Set generic toolbar properties
	SendMessage(m_tool_bar, TB_BUTTONSTRUCTSIZE, static_cast<WPARAM>(sizeof(TBBUTTON)), 0);
	SendMessage(m_tool_bar, TB_SETBITMAPSIZE, 0, MAKELPARAM(icon_cx, 16));

	// Set the bitmaps.
	HBITMAP bitmap = static_cast<HBITMAP>(LoadImage(m_inst, MAKEINTRESOURCE(bitmap_id), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
	TBADDBITMAP bitmap_info{nullptr, reinterpret_cast<UINT_PTR>(bitmap)};
	SendMessage(m_tool_bar, TB_ADDBITMAP, ctIcon, reinterpret_cast<LPARAM>(&bitmap_info));

	// Add buttons and resize to fit
	SendMessage(m_tool_bar, TB_ADDBUTTONS, ctIcon, reinterpret_cast<LPARAM>(tool_bar_button));
	SendMessage(m_tool_bar, TB_AUTOSIZE, 0, 0);

	// Flat
	LONG_PTR tool_type = GetWindowLongPtr(m_tool_bar, GWL_STYLE) | TBSTYLE_FLAT;
	SetWindowLongPtr(m_tool_bar, GWL_STYLE, tool_type);
	InvalidateRect(m_tool_bar, nullptr, TRUE);
	SendMessage(m_tool_bar, TB_SETEXTENDEDSTYLE, 0, static_cast<LPARAM>(TBSTYLE_EX_DRAWDDARROWS));

	HWND tool_tip = CreateWindowEx(0, // Extended window style
		TOOLTIPS_CLASS, // Class name
		nullptr,        // Window name
		TTS_ALWAYSTIP,  // Window style
		CW_USEDEFAULT,  // X Coordinate
		CW_USEDEFAULT,  // Y Coordinare
		CW_USEDEFAULT,  // Width
		CW_USEDEFAULT,  // Height
		m_window,       // Handle of parent window
		nullptr,        // Menu handle
		m_inst,         // Instance handle
		nullptr);       // WM_CREATE Data

	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = m_tool_bar;
	ti.hinst = m_inst;
	ti.uFlags = TTF_SUBCLASS;

	for (int i = 0; i < ctIcon; i++)
	{
		SendMessage(m_tool_bar, TB_GETITEMRECT, i, reinterpret_cast<LPARAM>(&ti.rect));
		ti.uId = tool_bar_button[i].idCommand;
		ti.lpszText = reinterpret_cast<LPTSTR>(tool_bar_button[i].idCommand);
		SendMessage(tool_tip, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));
	}

	SendMessage(m_tool_bar, TB_SETTOOLTIPS, reinterpret_cast<WPARAM>(tool_tip), 0);

	ShowWindow(m_tool_bar, TRUE);
	return m_tool_bar;
}

void CToolBar::SetWindowPos()
{
	SendMessage(m_tool_bar, WM_SIZE, 0, 0);
}

HWND CToolBar::GetHandle() const
{
	return m_window;
}

HWND CToolBar::GetCtrlHandle() const
{
	return m_tool_bar;
}
