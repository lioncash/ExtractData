#include "StdAfx.h"
#include "UI/Ctrl/UpDown.h"

HWND CUpDown::Create(HWND window, HWND buddy, int pos, UINT id, int upper, int lower, int x, int y, int cx, int cy)
{
	m_window = window;
	m_inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(window, GWLP_HINSTANCE));
	m_id = id;

	m_ctrl = CreateUpDownControl(
		WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_WRAP | UDS_SETBUDDYINT,
		x, y, cx, cy,
		window, id, m_inst, buddy,
		upper, lower, pos);
	return m_ctrl;
}
