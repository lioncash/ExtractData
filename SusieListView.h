#pragma once

#include "Ctrl/ListView.h"

class CSusieListView : public CListView
{
public:
	void Create(HWND hWnd, SOption& option, int x = 0, int y = 0, int cx = 0, int cy = 0);
	void Show();
	void Show(NMLVDISPINFO* pDispInfo);
	void ShowTip(LPNMLVGETINFOTIP ptip);
	BOOL CustomDraw(LPNMLVCUSTOMDRAW plvcd);
	void CreateMenu(LPARAM lp);

	BOOL SetCheck();
	void SetCheckAll(BOOL flag);

	SSusieInfo* GetFocusSusieInfo() { return &m_SusieInfo; }


private:
	SSusieInfo m_SusieInfo;
};
