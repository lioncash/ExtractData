#pragma once

#include "UI/WindowBase.h"

class CExistsDialog : public CWindowBase
{
public:
	void DoModal(HWND hWnd, LPCTSTR file_path);
	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) override;
	DWORD& GetOverWrite() { return m_overwrite; }

private:
	static DWORD m_overwrite;
	LPCTSTR m_file_path;
};
