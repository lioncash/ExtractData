#include "StdAfx.h"
#include "UI/Ctrl/ListView.h"

#include "Common.h"

SORTPARAM* CListView::m_sort_ptr;

CListView::CListView()
{
}

CListView::~CListView()
{
	Close();
}

void CListView::Init(HWND window, SOption& option)
{
	m_window = window;
	m_inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(window, GWLP_HINSTANCE));
	m_option = &option;
}

HWND CListView::Create(UINT id, std::vector<LVCOLUMN> columns, int x, int y, int cx, int cy)
{
	m_id = id;

	// Create listview
	HWND list = CreateWindowEx(WS_EX_CLIENTEDGE,
		WC_LISTVIEW, _T(""),
		WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA,
		x, y, cx, cy,
		m_window,
		reinterpret_cast<HMENU>(id),
		m_inst,
		nullptr);
	m_list = list;

	// Set style
	DWORD style = ListView_GetExtendedListViewStyle(list);
	style |= LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP;
	ListView_SetExtendedListViewStyle(list, style);

	// Make line spacing 16px
	m_image = ImageList_Create(1, 16, ILC_COLOR, 0, 0);
	ListView_SetImageList(list, m_image, LVSIL_STATE);

	YCIni ini(SBL_STR_INI_EXTRACTDATA);
	ini.SetSection(id);

	// Add a column header

	for (size_t i = 0 ; i < columns.size() ; i++)
	{
		TCHAR column_key[256];

		_stprintf(column_key, _T("column%zu"), i);

		ini.SetKey(column_key);
		ini.ReadDec(&columns[i].cx);

		columns[i].iSubItem = i;

		ListView_InsertColumn(list, i, &columns[i]);
	}

	return m_list;
}

void CListView::SetBkColor()
{
	//ListView_SetBkColor(m_list, m_option->ListBkColor);
	//ListView_SetTextBkColor(m_list, m_option->ListBkColor);
	ListView_SetBkColor(m_list, RGB((m_option->ListBkColor >> 16) & 0xFF, (m_option->ListBkColor >> 8) & 0xFF, m_option->ListBkColor & 0xFF));
	ListView_SetTextBkColor(m_list, RGB((m_option->ListBkColor >> 16) & 0xFF, (m_option->ListBkColor >> 8) & 0xFF, m_option->ListBkColor & 0xFF));
}

void CListView::SetTextColor()
{
	//ListView_SetTextColor(m_list, m_option->ListTextColor);
	ListView_SetTextColor(m_list, RGB((m_option->ListTextColor >> 16) & 0xFF, (m_option->ListTextColor >> 8) & 0xFF, m_option->ListTextColor & 0xFF));
}

void CListView::Sort(int column)
{
	m_sort.column = column;
	m_sort.direction ^= 1;
	m_sort_ptr = &m_sort;
	OnSort();
	InvalidateRect(m_list, nullptr, FALSE);
}

void CListView::Enable(BOOL flag)
{
	EnableWindow(m_list, flag);
}

void CListView::SetItemSelAll(UINT flag)
{
	ListView_SetItemState(m_list, -1, flag, LVIS_SELECTED);
}

void CListView::SetFocus()
{
	::SetFocus(m_list);
}

void CListView::SetWindowPos(int x, int y, int cx, int cy)
{
	MoveWindow(m_list, x, y, cx, cy, TRUE);
}

void CListView::SaveIni()
{
	YCIni ini(SBL_STR_INI_EXTRACTDATA);
	ini.SetSection(m_id);

	// Save column width
	const int columns = Header_GetItemCount(ListView_GetHeader(m_list));

	for (int i = 0 ; i < columns ; i++)
	{
		TCHAR column_name[256];
		_stprintf(column_name, _T("Column%d"), i);

		ini.SetKey(column_name);
		ini.WriteDec(ListView_GetColumnWidth(m_list, i));
	}
}

void CListView::Close()
{
	if (m_image)
	{
		ImageList_Destroy(m_image);
		m_image = nullptr;
	}
}

void CListView::Update()
{
	RECT rc;
	GetClientRect(m_list, &rc);
	InvalidateRect(m_list, &rc, FALSE);
}

UINT CListView::GetCountSel() const
{
	return ListView_GetSelectedCount(m_list);
}

INT CListView::GetCount() const
{
	return ListView_GetItemCount(m_list);
}

HWND CListView::GetHandle() const
{
	return m_list;
}

int CListView::GetNextItem(int item) const
{
	return ListView_GetNextItem(m_list, item, LVNI_ALL | LVNI_SELECTED);
}

int CListView::GetFocusItem() const
{
	return ListView_GetNextItem(m_list, -1, LVNI_ALL | LVNI_FOCUSED);
}
