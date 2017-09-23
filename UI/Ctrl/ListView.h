#pragma once

struct SOption;

// When you sort the list of parameters
struct SORTPARAM
{
	int column;
	BOOL direction; // 0 = ascending order, 1 = descending order
};

class CListView
{
public:
	CListView();
	virtual ~CListView();

	void Init(HWND window, SOption& option);
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
	INT  GetCount() const;
	HWND GetHandle() const;

	int GetNextItem(int nItem) const;
	int GetFocusItem() const;

protected:
	HWND m_window = nullptr;
	HWND m_list = nullptr;
	HINSTANCE m_inst = nullptr;
	SOption* m_option = nullptr;
	SORTPARAM m_sort{};
	static SORTPARAM* m_sort_ptr;

	HWND Create(UINT id, std::vector<LVCOLUMN> columns, int x = 0, int y = 0, int cx = 0, int cy = 0);

	template <typename TYPE>
	static bool retCompare(TYPE a, TYPE b)
	{
		return m_sort_ptr->direction ? (a > b) : (a < b);
	}

	virtual void OnSort() {}

private:
	HIMAGELIST m_image = nullptr;
	UINT m_id = 0;
};
