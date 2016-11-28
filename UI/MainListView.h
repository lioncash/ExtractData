#pragma once

#include "Common.h"
#include "UI/Ctrl/ListView.h"

class CMainListView : public CListView
{
public:
	void Create(HWND hWnd, SOption& option);
	void Show();
	void Show(NMLVDISPINFO* pDispInfo);
	void ShowTip(LPNMLVGETINFOTIP ptip);
	void Clear();

	virtual void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);

	std::vector<SFileInfo>& GetFileInfo() { return m_ent; }

protected:
	void OnSort() override;

private:
	std::vector<SFileInfo> m_ent;
	static bool CompareFunc(const SFileInfo& a, const SFileInfo& b);
};
