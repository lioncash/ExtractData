#pragma once

class CUpDown
{
private:
	HWND      m_hWnd;
	HINSTANCE m_hInst;
	HWND      m_hCtrl;
	UINT      m_ID;

public:
	HWND Create(HWND hWnd, HWND hBuddy, int nPos, UINT ID, int nUpper, int nLower, int x = 0, int y = 0, int cx = 0, int cy = 0);
};
