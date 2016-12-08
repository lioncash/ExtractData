#pragma once

#include "Susie.h"
#include "UI/Ctrl/ListView.h"

struct SOption;

class CSusieListView : public CListView
{
public:
	void Create(HWND hWnd, SOption& option, int x = 0, int y = 0, int cx = 0, int cy = 0);
	void Show();
	void Show(NMLVDISPINFO* disp_info);
	void ShowTip(LPNMLVGETINFOTIP tip);
	BOOL CustomDraw(LPNMLVCUSTOMDRAW custom_draw);
	void CreateMenu(LPARAM param);

	bool SetCheck();
	void SetCheckAll(bool flag);

	SSusieInfo* GetFocusSusieInfo() { return &m_susie_info; }

private:
	SSusieInfo m_susie_info;
};
