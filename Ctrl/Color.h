#pragma once

class CColor
{
private:
	CHOOSECOLOR m_cc;
	static COLORREF m_CustColors[16];

public:
	void Init(COLORREF color);
	BOOL DoModal(HWND hWnd);

	COLORREF GetColor() { return m_cc.rgbResult; }
};
