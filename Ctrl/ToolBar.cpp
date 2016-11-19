#include "StdAfx.h"
#include "../res/ResExtractData.h"
#include "Common.h"
#include "Toolbar.h"

CToolBar::CToolBar()
{
}

void CToolBar::Init(HWND hWnd)
{
	m_hWnd = hWnd;
	m_hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
	LoadIni();
}

// Function that creates the toolbar
HWND CToolBar::Create(HWND hWnd, LPTBBUTTON tbButton, UINT BmpID, int icon_cx, int ctIcon)
{
	Init(hWnd);

	HBITMAP hBitmap = static_cast<HBITMAP>(LoadImage(m_hInst, MAKEINTRESOURCE(BmpID), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));

	HWND hToolBar = CreateToolbarEx(
		m_hWnd,
		WS_CHILD | WS_VISIBLE | CCS_NORESIZE | TBSTYLE_TOOLTIPS,
		ID_TOOLBAR,
		ctIcon,     // Number of images
		nullptr,
		reinterpret_cast<UINT_PTR>(hBitmap),
		tbButton,
		ctIcon,     // Number of buttons
		0,          // Width of the button
		0,          // Height of the button
		icon_cx,    // Width of the image, 
		16,         // Height of the image
		sizeof(*tbButton));

	m_hToolBar = hToolBar;

	// Flat
	LONG ToolType = GetWindowLong(hToolBar, GWL_STYLE) | TBSTYLE_FLAT;
	SetWindowLong(hToolBar, GWL_STYLE, ToolType);
	InvalidateRect(hToolBar, nullptr, TRUE);
	SendMessage(hToolBar, TB_SETEXTENDEDSTYLE, 0, static_cast<LPARAM>(TBSTYLE_EX_DRAWDDARROWS));

	HWND hToolTip = CreateWindowEx(0, //Extended window style
		TOOLTIPS_CLASS, // Class name
		nullptr,        // Window name
		TTS_ALWAYSTIP,  // Window style
		CW_USEDEFAULT,  // X Coordinate
		CW_USEDEFAULT,  // Y Coordinare
		CW_USEDEFAULT,  // Width
		CW_USEDEFAULT,  // Height
		m_hWnd,         // Handle of parent window
		nullptr,        // Menu handle
		m_hInst,        // Instance handle
		nullptr);       // WM_CREATE Data

	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hToolBar;
	ti.hinst = m_hInst;
	ti.uFlags = TTF_SUBCLASS;

	for (int i = 0; i < ctIcon; i++)
	{
		SendMessage(hToolBar, TB_GETITEMRECT, i, reinterpret_cast<LPARAM>(&ti.rect));
		ti.uId = tbButton[i].idCommand;
		ti.lpszText = reinterpret_cast<LPTSTR>(tbButton[i].idCommand);
		SendMessage(hToolTip, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));
	}

	SendMessage(hToolBar, TB_SETTOOLTIPS, reinterpret_cast<WPARAM>(hToolTip), 0);

	return hToolBar;
}
