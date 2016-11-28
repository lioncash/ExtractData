#pragma once

#include "UI/Ctrl/CtrlBase.h"

class CRadioBtn : public CCtrlBase
{
public:
	HWND Create(HWND hWnd, LPCTSTR lpCaption, UINT uID, int x = 0, int y = 0, int cx = 0, int cy = 0) override;

	void Close();
	void SetCheck(BOOL flag);
	void SetCheck(DWORD num, BOOL flag);
	BOOL GetCheck() const;
	BOOL GetCheck(DWORD num) const;
	UINT GetID(DWORD num) const;

private:
	std::vector<HWND> m_hRadios;
	std::vector<UINT> m_IDs;
};
