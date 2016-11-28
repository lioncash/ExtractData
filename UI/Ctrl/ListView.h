#pragma once

struct SOption;

#ifndef ListView_SetCheckState
	#define ListView_SetCheckState(hwndLV, i, fCheck) \
	        ListView_SetItemState(hwndLV, i, INDEXTOSTATEIMAGEMASK((fCheck)+1), LVIS_STATEIMAGEMASK)
#endif

// When you sort the list of parameters
struct SORTPARAM
{
	int column;
	BOOL direct; // 0 = ascending order, 1 = descending order
};

class CListView
{
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
	void SetItemSelAll(UINT flag);
	void SetFocus();
	void SetWindowPos(int x, int y, int cx, int cy);
	void Update();

	UINT GetCountSel() const;
	INT  GetCount()    const;
	HWND GetHandle()   const;

	int GetNextItem(int nItem) const;
	int GetFocusItem()         const;

protected:
	HWND m_hWnd = nullptr;
	HWND m_hList = nullptr;
	HINSTANCE m_hInst = nullptr;
	SOption* m_pOption = nullptr;
	SORTPARAM m_sort{};
	static SORTPARAM* m_pSort;

	HWND Create(UINT uID, std::vector<LVCOLUMN>& lvcols, int x = 0, int y = 0, int cx = 0, int cy = 0);

	template <typename TYPE>
	static bool retCompare(TYPE a, TYPE b)
	{
		return ((m_pSort->direct) ? (a > b) : (a < b));
	}

	virtual void OnSort() {}

private:
	HIMAGELIST m_hImage = nullptr;
	UINT m_uID = 0;
};
