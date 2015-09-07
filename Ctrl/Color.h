#pragma once

class CColor
{
public:
	void Init(COLORREF color);
	BOOL DoModal(HWND hWnd);

	COLORREF GetColor() { return m_cc.rgbResult; }

private:
	CHOOSECOLOR m_cc;
	static COLORREF m_CustColors[16];
};
