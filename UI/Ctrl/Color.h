#pragma once

class CColor
{
public:
	void Init(COLORREF color);
	BOOL DoModal(HWND hWnd);

	COLORREF GetColor() const;

private:
	CHOOSECOLOR m_cc;
	static COLORREF m_CustColors[16];
};
