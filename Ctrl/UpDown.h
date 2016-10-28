#pragma once

class CUpDown
{
public:
	HWND Create(HWND hWnd, HWND hBuddy, int nPos, UINT ID, int nUpper, int nLower, int x = 0, int y = 0, int cx = 0, int cy = 0);

private:
	HWND      m_hWnd = nullptr;
	HINSTANCE m_hInst = nullptr;
	HWND      m_hCtrl = nullptr;
	UINT      m_ID = 0;
};
