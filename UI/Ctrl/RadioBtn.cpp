#include "StdAfx.h"
#include "UI/Ctrl/RadioBtn.h"

HWND CRadioBtn::Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x, int y, int cx, int cy)
{
	Init(hWnd, ID);
	DWORD Group = (m_hRadios.empty()) ? WS_GROUP : 0;
	HWND hRadio = CreateCtrl(_T("BUTTON"), BS_AUTORADIOBUTTON | WS_TABSTOP | Group, lpCaption, x, y, cx, cy);
	m_hRadios.push_back(hRadio);
	m_IDs.push_back(ID);
	return hRadio;
}
