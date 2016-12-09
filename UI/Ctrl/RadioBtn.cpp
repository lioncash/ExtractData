#include "StdAfx.h"
#include "UI/Ctrl/RadioBtn.h"

HWND CRadioBtn::Create(HWND window, LPCTSTR caption, UINT id, int x, int y, int cx, int cy)
{
	Init(window, id);
	const DWORD group = (m_radios.empty()) ? WS_GROUP : 0;
	HWND radio = CreateCtrl(_T("BUTTON"), BS_AUTORADIOBUTTON | WS_TABSTOP | group, caption, x, y, cx, cy);
	m_radios.push_back(radio);
	m_ids.push_back(id);
	return radio;
}

void CRadioBtn::Close()
{
	m_radios.clear();
	m_ids.clear();
}

void CRadioBtn::SetCheck(BOOL flag)
{
	Button_SetCheck(GetCtrlHandle(), flag);
}

void CRadioBtn::SetCheck(DWORD num, BOOL flag)
{
	Button_SetCheck(m_radios[num], flag);
}

BOOL CRadioBtn::GetCheck() const
{
	return Button_GetCheck(GetCtrlHandle());
}

BOOL CRadioBtn::GetCheck(DWORD num) const
{
	return Button_GetCheck(m_radios[num]);
}

UINT CRadioBtn::GetID(DWORD num) const
{
	return (num >= m_ids.size()) ? 0 : m_ids[num];
}
