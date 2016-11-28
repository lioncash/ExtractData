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

void CRadioBtn::Close()
{
	m_hRadios.clear();
	m_IDs.clear();
}

void CRadioBtn::SetCheck(BOOL flag)
{
	Button_SetCheck(GetCtrlHandle(), flag);
}

void CRadioBtn::SetCheck(DWORD num, BOOL flag)
{
	Button_SetCheck(m_hRadios[num], flag);
}

BOOL CRadioBtn::GetCheck() const
{
	return Button_GetCheck(GetCtrlHandle());
}

BOOL CRadioBtn::GetCheck(DWORD num) const
{
	return Button_GetCheck(m_hRadios[num]);
}

UINT CRadioBtn::GetID(DWORD num) const
{
	return (num >= m_IDs.size()) ? 0 : m_IDs[num];
}
