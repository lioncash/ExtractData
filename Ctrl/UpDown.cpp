#include "stdafx.h"
#include "UpDown.h"

HWND CUpDown::Create(HWND hWnd, HWND hBuddy, int nPos, UINT ID, int nUpper, int nLower, int x, int y, int cx, int cy)
{
	m_hWnd = hWnd;
	m_hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
	m_ID = ID;

	m_hCtrl = CreateUpDownControl(
		WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_WRAP | UDS_SETBUDDYINT,
		x, y, cx, cy,
		hWnd, ID, m_hInst, hBuddy,
		nUpper, nLower, nPos);
	return m_hCtrl;
}
