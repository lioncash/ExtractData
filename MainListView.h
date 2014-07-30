#pragma once

#include "Ctrl/ListView.h"

class CMainListView : public CListView
{
private:
	std::vector<SFileInfo> m_ent;
	static BOOL CompareFunc(const SFileInfo& a, const SFileInfo& b);

protected:
	void         OnSort();

public:
	void         Create(HWND hWnd, SOption& option);
	void         Show();
	void         Show(NMLVDISPINFO* pDispInfo);
	void         ShowTip(LPNMLVGETINFOTIP ptip);
	void         Clear();

	virtual void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);

	std::vector<SFileInfo>& GetFileInfo() { return m_ent; }
};
