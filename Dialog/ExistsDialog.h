#pragma once

#include "../WindowBase.h"

class CExistsDialog : public CWindowBase
{
public:
	void DoModal(HWND hWnd, LPCTSTR pFilePath);
	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) override;
	DWORD& GetOverWrite() { return (m_fOverWrite); }

private:
	static DWORD m_fOverWrite;
	LPCTSTR m_pFilePath;
};
