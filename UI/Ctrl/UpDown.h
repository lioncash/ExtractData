#pragma once

class CUpDown
{
public:
	HWND Create(HWND window, HWND buddy, int pos, UINT id, int upper, int lower, int x = 0, int y = 0, int cx = 0, int cy = 0);

private:
	HWND      m_window = nullptr;
	HINSTANCE m_inst = nullptr;
	HWND      m_ctrl = nullptr;
	UINT      m_id = 0;
};
