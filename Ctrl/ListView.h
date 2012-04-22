#pragma once

#include "../Common.h"

#ifndef ListView_SetCheckState
   #define ListView_SetCheckState(hwndLV, i, fCheck) \
      ListView_SetItemState(hwndLV, i, \
      INDEXTOSTATEIMAGEMASK((fCheck)+1), LVIS_STATEIMAGEMASK)
#endif

// When you sort the list of parameters
struct SORTPARAM {
	int column;
	BOOL direct; // 0 = ascending order, 1 = descending order
};

class CListView {
private:
	HIMAGELIST m_hImage;
	UINT m_uID;

protected:
	HWND m_hWnd;
	HWND m_hList;
	HINSTANCE m_hInst;
	SOption* m_pOption;
	SORTPARAM m_sort;
	static SORTPARAM* m_pSort;

	HWND Create(UINT uID, std::vector<LVCOLUMN>& lvcols, int x = 0, int y = 0, int cx = 0, int cy = 0);
	template <typename TYPE> static BOOL retCompare(TYPE a, TYPE b) { return ((m_pSort->direct) ? (a > b) : (a < b)); }
	virtual void OnSort() {}

public:
	CListView();
	virtual ~CListView();

	void Init(HWND hWnd, SOption& option);
	void SaveIni();
	void Close();

	void SetBkColor();
	void SetTextColor();

	void Sort(int column);
	void Enable(BOOL flag);
	void SetItemSelAll(UINT flag) { ListView_SetItemState(m_hList, -1, flag, LVIS_SELECTED); }
	void SetFocus() { ::SetFocus(m_hList); }
	void SetWindowPos(int x, int y, int cx, int cy) { MoveWindow(m_hList, x, y, cx, cy, TRUE); }
	void Update();

	UINT	GetCountSel()	{ return (ListView_GetSelectedCount(m_hList)); }
	INT		GetCount()		{ return (ListView_GetItemCount(m_hList)); }
	HWND	GetHandle()		{ return (m_hList); }

	int	GetNextItem(int nItem) { return ListView_GetNextItem(m_hList, nItem, LVNI_ALL | LVNI_SELECTED); }
	int GetFocusItem()	{ return ListView_GetNextItem(m_hList, -1, LVNI_ALL | LVNI_FOCUSED); }
};
