#pragma once

class CColor
{
public:
	void Init(COLORREF color);
	BOOL DoModal(HWND window);

	COLORREF GetColor() const;

private:
	CHOOSECOLOR m_cc;
};
