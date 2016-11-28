#include "StdAfx.h"
#include "UI/WindowBase.h"

#include "Common.h"

CWindowBase::CWindowBase()
{
}

CWindowBase::~CWindowBase()
{
	RemoveProp(m_hWnd, _T("CWindowBase"));
}

void CWindowBase::Init()
{
	MoveWindowCenter();
}

void CWindowBase::Init(LONG cx, LONG cy)
{
	MoveWindowCenter(cx, cy);
}

void CWindowBase::Init(UINT uID, LONG cx, LONG cy)
{
	m_uID = uID;

	YCIni clIni(SBL_STR_INI_EXTRACTDATA);
	clIni.SetSection(m_uID);

	RECT rc;
	SetRect(&rc, 0, 0, cx, cy);
	POINT pt = GetCenterPt(rc);

	clIni.SetKey(_T("Left"));
	clIni.ReadDec(&pt.x);
	clIni.SetKey(_T("Top"));
	clIni.ReadDec(&pt.y);
	clIni.SetKey(_T("Width"));
	clIni.ReadDec(&cx);
	clIni.SetKey(_T("Height"));
	clIni.ReadDec(&cy);

	WINDOWPLACEMENT wndpl = {};
	wndpl.length = sizeof(WINDOWPLACEMENT);
	wndpl.showCmd = SW_HIDE;

	SetRect(&wndpl.rcNormalPosition, pt.x, pt.y, pt.x + cx, pt.y + cy);
	SetWindowPlacement(m_hWnd, &wndpl);

	clIni.SetKey(_T("showCmd"));
	clIni.ReadDec<UINT>(&wndpl.showCmd, SW_SHOWNORMAL);

	ShowWindow(m_hWnd, wndpl.showCmd);
}

void CWindowBase::Init(HWND hWnd)
{
	MoveWindowCenter(hWnd);
}

void CWindowBase::Init(HWND hWnd, LONG cx, LONG cy)
{
	MoveWindowCenter(hWnd, cx, cy);
}

void CWindowBase::SaveIni()
{
	YCIni clIni(SBL_STR_INI_EXTRACTDATA);
	clIni.SetSection(m_uID);

	WINDOWPLACEMENT wndpl;

	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(m_hWnd, &wndpl);

	// Save Position
	clIni.SetKey(_T("Left"));
	clIni.WriteDec(wndpl.rcNormalPosition.left);
	clIni.SetKey(_T("Top"));
	clIni.WriteDec(wndpl.rcNormalPosition.top);

	// Save Size
	clIni.SetKey(_T("Width"));
	clIni.WriteDec(wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left);
	clIni.SetKey(_T("Height"));
	clIni.WriteDec(wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top);

	if (wndpl.showCmd != SW_SHOWMINIMIZED)
	{
		clIni.SetKey(_T("showCmd"));
		clIni.WriteDec(wndpl.showCmd);
	}
}

bool CWindowBase::Attach(HWND hWnd)
{
	if (!hWnd)
		return false;

	m_hWnd = hWnd;
	m_hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));

	// Determine window or dialog
	m_bDialog = GetWindowLongPtr(hWnd, DWL_DLGPROC) != 0;
	const int tproc = m_bDialog ? DWL_DLGPROC : GWL_WNDPROC;

	SetProp(m_hWnd, _T("CWindowBase"), static_cast<HANDLE>(this));

	// Subclass an existing window
	if (GetWindowLongPtr(m_hWnd, tproc) != reinterpret_cast<LONG_PTR>(WndStaticProc))
		m_oldWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_hWnd, tproc, reinterpret_cast<LONG_PTR>(WndStaticProc)));

	return true;
}

bool CWindowBase::Detach()
{
	if (!m_hWnd)
		return false;

	// Remove the subclass
	if (m_oldWndProc)
	{
		const int tproc = m_bDialog ? DWL_DLGPROC : GWL_WNDPROC;
		SetWindowLongPtr(m_hWnd, tproc, reinterpret_cast<LONG_PTR>(m_oldWndProc));
	}

	RemoveProp(m_hWnd, _T("CWindowBase"));
	return true;
}

LRESULT CALLBACK CWindowBase::WndStaticProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// Retrieve data from the property list
	CWindowBase* tWnd = static_cast<CWindowBase*>(GetProp(hWnd, _T("CWindowBase")));

	// Could not be obtained during processing
	if (tWnd == nullptr)
	{
		if ((msg == WM_CREATE) || (msg == WM_NCCREATE))
			tWnd = static_cast<CWindowBase*>(reinterpret_cast<LPCREATESTRUCT>(lp)->lpCreateParams);
		else if (msg == WM_INITDIALOG)
			tWnd = reinterpret_cast<CWindowBase*>(lp);

		if (tWnd)
			tWnd->Attach(hWnd);
	}

	// Success getting the tWnd
	if (tWnd)
	{
		LRESULT lResult = tWnd->WndProc(hWnd, msg, wp, lp);
		if (msg == WM_DESTROY)
			tWnd->Detach();
		return lResult;
	}

	// If dialog, returns FALSE
	if (GetWindowLongPtr(hWnd, DWL_DLGPROC))
		return FALSE;

	return DefWindowProc(hWnd, msg, wp, lp);
}

LRESULT CWindowBase::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// If any subclasses of old window processing are left.
	if (m_oldWndProc)
		return CallWindowProc(m_oldWndProc, hWnd, msg, wp, lp);
	// If there is a dialog, return FALSE
	if (m_bDialog)
		return FALSE;
	// If it is not subclassed, call the default window procedure
	return DefWindowProc(hWnd, msg, wp, lp);
}

void CWindowBase::MoveWindowCenter()
{
	RECT rc;
	GetWindowRect(m_hWnd, &rc);
	POINT pt = GetCenterPt(rc);
	MoveWindow(m_hWnd, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

void CWindowBase::MoveWindowCenter(LONG cx, LONG cy)
{
	RECT rc;
	SetRect(&rc, 0, 0, cx, cy);
	POINT pt = GetCenterPt(rc);
	MoveWindow(m_hWnd, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

void CWindowBase::MoveWindowCenter(HWND hWnd)
{
	RECT rc;
	GetWindowRect(hWnd, &rc);
	POINT pt = GetCenterPt(hWnd, rc);
	MoveWindow(hWnd, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

void CWindowBase::MoveWindowCenter(HWND hWnd, LONG cx, LONG cy)
{
	RECT rc;
	SetRect(&rc, 0, 0, cx, cy);
	POINT pt = GetCenterPt(hWnd, rc);
	MoveWindow(hWnd, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

POINT CWindowBase::GetCenterPt(RECT& dlgrc)
{
	RECT ParentRect;
	HWND hParentWnd = GetParent(m_hWnd);
	if (hParentWnd == nullptr)
		hParentWnd = GetDesktopWindow();
	GetWindowRect(hParentWnd, &ParentRect);
	//CError error;
	//error.Message(m_hWnd, _T("%d %d %d %d"), ParentRect.left, ParentRect.top, ParentRect.right, ParentRect.bottom);

	POINT pt;
	pt.x = ParentRect.left + ((ParentRect.right - ParentRect.left) - (dlgrc.right - dlgrc.left)) / 2;
	pt.y = ParentRect.top + ((ParentRect.bottom - ParentRect.top) - (dlgrc.bottom - dlgrc.top)) / 2;
	//error.Message(m_hWnd, _T("%d %d %d %d"), dlgrc.left, dlgrc.top, dlgrc.right, dlgrc.bottom);
	return pt;
}

POINT CWindowBase::GetCenterPt(HWND hWnd, RECT& dlgrc)
{
	RECT ParentRect;
	HWND hParentWnd = GetParent(hWnd);
	if (hParentWnd == nullptr)
		hParentWnd = GetDesktopWindow();
	GetWindowRect(hParentWnd, &ParentRect);
	//CError error;
	//error.Message(m_hWnd, _T("%d %d %d %d"), ParentRect.left, ParentRect.top, ParentRect.right, ParentRect.bottom);

	POINT pt;
	pt.x = ParentRect.left + ((ParentRect.right - ParentRect.left) - (dlgrc.right - dlgrc.left)) / 2;
	pt.y = ParentRect.top + ((ParentRect.bottom - ParentRect.top) - (dlgrc.bottom - dlgrc.top)) / 2;
	//error.Message(m_hWnd, _T("%d %d %d %d"), dlgrc.left, dlgrc.top, dlgrc.right, dlgrc.bottom);
	return pt;
}
