#include "StdAfx.h"
#include "UI/WindowBase.h"

#include "Common.h"

CWindowBase::CWindowBase()
{
}

CWindowBase::~CWindowBase()
{
	RemoveProp(m_window, _T("CWindowBase"));
}

void CWindowBase::Init()
{
	MoveWindowCenter();
}

void CWindowBase::Init(LONG cx, LONG cy)
{
	MoveWindowCenter(cx, cy);
}

void CWindowBase::Init(UINT id, LONG cx, LONG cy)
{
	m_id = id;

	YCIni ini(SBL_STR_INI_EXTRACTDATA);
	ini.SetSection(m_id);

	RECT rc;
	SetRect(&rc, 0, 0, cx, cy);
	POINT pt = GetCenterPt(rc);

	ini.SetKey(_T("Left"));
	ini.ReadDec(&pt.x);
	ini.SetKey(_T("Top"));
	ini.ReadDec(&pt.y);
	ini.SetKey(_T("Width"));
	ini.ReadDec(&cx);
	ini.SetKey(_T("Height"));
	ini.ReadDec(&cy);

	WINDOWPLACEMENT wndpl = {};
	wndpl.length = sizeof(WINDOWPLACEMENT);
	wndpl.showCmd = SW_HIDE;

	SetRect(&wndpl.rcNormalPosition, pt.x, pt.y, pt.x + cx, pt.y + cy);
	SetWindowPlacement(m_window, &wndpl);

	ini.SetKey(_T("showCmd"));
	ini.ReadDec<UINT>(&wndpl.showCmd, SW_SHOWNORMAL);

	ShowWindow(m_window, wndpl.showCmd);
}

void CWindowBase::Init(HWND window)
{
	MoveWindowCenter(window);
}

void CWindowBase::Init(HWND window, LONG cx, LONG cy)
{
	MoveWindowCenter(window, cx, cy);
}

void CWindowBase::SaveIni()
{
	YCIni ini(SBL_STR_INI_EXTRACTDATA);
	ini.SetSection(m_id);

	WINDOWPLACEMENT wndpl;

	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(m_window, &wndpl);

	// Save Position
	ini.SetKey(_T("Left"));
	ini.WriteDec(wndpl.rcNormalPosition.left);
	ini.SetKey(_T("Top"));
	ini.WriteDec(wndpl.rcNormalPosition.top);

	// Save Size
	ini.SetKey(_T("Width"));
	ini.WriteDec(wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left);
	ini.SetKey(_T("Height"));
	ini.WriteDec(wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top);

	if (wndpl.showCmd != SW_SHOWMINIMIZED)
	{
		ini.SetKey(_T("showCmd"));
		ini.WriteDec(wndpl.showCmd);
	}
}

bool CWindowBase::Attach(HWND window)
{
	if (!window)
		return false;

	m_window = window;
	m_inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(window, GWLP_HINSTANCE));

	// Determine window or dialog
	m_is_dialog = GetWindowLongPtr(window, DWL_DLGPROC) != 0;
	const int tproc = m_is_dialog ? DWL_DLGPROC : GWL_WNDPROC;

	SetProp(m_window, _T("CWindowBase"), static_cast<HANDLE>(this));

	// Subclass an existing window
	if (GetWindowLongPtr(m_window, tproc) != reinterpret_cast<LONG_PTR>(WndStaticProc))
		m_old_window_proc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_window, tproc, reinterpret_cast<LONG_PTR>(WndStaticProc)));

	return true;
}

bool CWindowBase::Detach()
{
	if (!m_window)
		return false;

	// Remove the subclass
	if (m_old_window_proc)
	{
		const int tproc = m_is_dialog ? DWL_DLGPROC : GWL_WNDPROC;
		SetWindowLongPtr(m_window, tproc, reinterpret_cast<LONG_PTR>(m_old_window_proc));
	}

	RemoveProp(m_window, _T("CWindowBase"));
	return true;
}

LRESULT CALLBACK CWindowBase::WndStaticProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
	// Retrieve data from the property list
	CWindowBase* tWnd = static_cast<CWindowBase*>(GetProp(window, _T("CWindowBase")));

	// Could not be obtained during processing
	if (tWnd == nullptr)
	{
		if ((msg == WM_CREATE) || (msg == WM_NCCREATE))
			tWnd = static_cast<CWindowBase*>(reinterpret_cast<LPCREATESTRUCT>(lp)->lpCreateParams);
		else if (msg == WM_INITDIALOG)
			tWnd = reinterpret_cast<CWindowBase*>(lp);

		if (tWnd)
			tWnd->Attach(window);
	}

	// Success getting the tWnd
	if (tWnd)
	{
		LRESULT lResult = tWnd->WndProc(window, msg, wp, lp);
		if (msg == WM_DESTROY)
			tWnd->Detach();
		return lResult;
	}

	// If dialog, returns FALSE
	if (GetWindowLongPtr(window, DWL_DLGPROC))
		return FALSE;

	return DefWindowProc(window, msg, wp, lp);
}

LRESULT CWindowBase::WndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
	// If any subclasses of old window processing are left.
	if (m_old_window_proc)
		return CallWindowProc(m_old_window_proc, window, msg, wp, lp);
	// If there is a dialog, return FALSE
	if (m_is_dialog)
		return FALSE;
	// If it is not subclassed, call the default window procedure
	return DefWindowProc(window, msg, wp, lp);
}

void CWindowBase::MoveWindowCenter()
{
	RECT rc;
	GetWindowRect(m_window, &rc);
	POINT pt = GetCenterPt(rc);
	MoveWindow(m_window, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

void CWindowBase::MoveWindowCenter(LONG cx, LONG cy)
{
	RECT rc;
	SetRect(&rc, 0, 0, cx, cy);
	POINT pt = GetCenterPt(rc);
	MoveWindow(m_window, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

void CWindowBase::MoveWindowCenter(HWND window)
{
	RECT rc;
	GetWindowRect(window, &rc);
	POINT pt = GetCenterPt(window, rc);
	MoveWindow(window, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

void CWindowBase::MoveWindowCenter(HWND window, LONG cx, LONG cy)
{
	RECT rc;
	SetRect(&rc, 0, 0, cx, cy);
	POINT pt = GetCenterPt(window, rc);
	MoveWindow(window, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

POINT CWindowBase::GetCenterPt(const RECT& dlgrc)
{
	RECT parent_rect;
	HWND parent_window = GetParent(m_window);
	if (parent_window == nullptr)
		parent_window = GetDesktopWindow();
	GetWindowRect(parent_window, &parent_rect);
	//CError error;
	//error.Message(m_window, _T("%d %d %d %d"), parent_rect.left, parent_rect.top, parent_rect.right, parent_rect.bottom);

	POINT pt;
	pt.x = parent_rect.left + ((parent_rect.right - parent_rect.left) - (dlgrc.right - dlgrc.left)) / 2;
	pt.y = parent_rect.top + ((parent_rect.bottom - parent_rect.top) - (dlgrc.bottom - dlgrc.top)) / 2;
	//error.Message(m_window, _T("%d %d %d %d"), dlgrc.left, dlgrc.top, dlgrc.right, dlgrc.bottom);
	return pt;
}

POINT CWindowBase::GetCenterPt(HWND window, const RECT& dlgrc)
{
	RECT parent_rect;
	HWND parent_window = GetParent(window);
	if (parent_window == nullptr)
		parent_window = GetDesktopWindow();
	GetWindowRect(parent_window, &parent_rect);
	//CError error;
	//error.Message(m_window, _T("%d %d %d %d"), parent_rect.left, parent_rect.top, parent_rect.right, parent_rect.bottom);

	POINT pt;
	pt.x = parent_rect.left + ((parent_rect.right - parent_rect.left) - (dlgrc.right - dlgrc.left)) / 2;
	pt.y = parent_rect.top + ((parent_rect.bottom - parent_rect.top) - (dlgrc.bottom - dlgrc.top)) / 2;
	//error.Message(m_window, _T("%d %d %d %d"), dlgrc.left, dlgrc.top, dlgrc.right, dlgrc.bottom);
	return pt;
}
