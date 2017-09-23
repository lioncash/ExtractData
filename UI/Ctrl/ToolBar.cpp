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

	HBITMAP bitmap = static_cast<HBITMAP>(LoadImage(m_inst, MAKEINTRESOURCE(bitmap_id), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));

	HWND tool_bar = CreateToolbarEx(
		m_window,
		WS_CHILD | WS_VISIBLE | CCS_NORESIZE | TBSTYLE_TOOLTIPS,
		ID_TOOLBAR,
		ctIcon,     // Number of images
		nullptr,
		reinterpret_cast<UINT_PTR>(bitmap),
		tool_bar_button,
		ctIcon,     // Number of buttons
		0,          // Width of the button
		0,          // Height of the button
		icon_cx,    // Width of the image, 
		16,         // Height of the image
		sizeof(*tool_bar_button));

	m_tool_bar = tool_bar;

	// Flat
	LONG_PTR tool_type = GetWindowLongPtr(tool_bar, GWL_STYLE) | TBSTYLE_FLAT;
	SetWindowLongPtr(tool_bar, GWL_STYLE, tool_type);
	InvalidateRect(tool_bar, nullptr, TRUE);
	SendMessage(tool_bar, TB_SETEXTENDEDSTYLE, 0, static_cast<LPARAM>(TBSTYLE_EX_DRAWDDARROWS));

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
	ti.hwnd = tool_bar;
	ti.hinst = m_inst;
	ti.uFlags = TTF_SUBCLASS;

	for (int i = 0; i < ctIcon; i++)
	{
		SendMessage(tool_bar, TB_GETITEMRECT, i, reinterpret_cast<LPARAM>(&ti.rect));
		ti.uId = tool_bar_button[i].idCommand;
		ti.lpszText = reinterpret_cast<LPTSTR>(tool_bar_button[i].idCommand);
		SendMessage(tool_tip, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));
	}

	SendMessage(tool_bar, TB_SETTOOLTIPS, reinterpret_cast<WPARAM>(tool_tip), 0);

	return tool_bar;
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
