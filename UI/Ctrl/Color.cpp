#include "StdAfx.h"
#include "UI/Ctrl/Color.h"

void CColor::Init(COLORREF color)
{
	m_cc.lStructSize = sizeof(CHOOSECOLOR);
	m_cc.rgbResult = color;
	m_cc.Flags = CC_RGBINIT;
}

BOOL CColor::DoModal(HWND hWnd)
{
	m_cc.hwndOwner = hWnd;
	return ChooseColor(&m_cc);
}

COLORREF CColor::GetColor() const
{
	return m_cc.rgbResult;
}
