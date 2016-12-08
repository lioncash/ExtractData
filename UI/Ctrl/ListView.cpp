#include "StdAfx.h"
#include "UI/Ctrl/ListView.h"

#include "Common.h"

SORTPARAM* CListView::m_pSort;

CListView::CListView()
{
}

CListView::~CListView()
{
	Close();
}

void CListView::Init(HWND hWnd, SOption& option)
{
	m_hWnd = hWnd;
	m_hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
	m_pOption = &option;
}

HWND CListView::Create(UINT id, std::vector<LVCOLUMN> columns, int x, int y, int cx, int cy)
{
	m_uID = id;

	// Create listview
	HWND list = CreateWindowEx(WS_EX_CLIENTEDGE,
		WC_LISTVIEW, _T(""),
		WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA,
		x, y, cx, cy,
		m_hWnd,
		reinterpret_cast<HMENU>(id),
		m_hInst,
		nullptr);
	m_hList = list;

	// Set style
	DWORD style = ListView_GetExtendedListViewStyle(list);
	style |= LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP;
	ListView_SetExtendedListViewStyle(list, style);

	// Make line spacing 16px
	m_hImage = ImageList_Create(1, 16, ILC_COLOR, 0, 0);
	ListView_SetImageList(list, m_hImage, LVSIL_STATE);

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

	return m_hList;
}

void CListView::SetBkColor()
{
	//ListView_SetBkColor(m_hList, m_pOption->ListBkColor);
	//ListView_SetTextBkColor(m_hList, m_pOption->ListBkColor);
	ListView_SetBkColor(m_hList, RGB((m_pOption->ListBkColor >> 16) & 0xFF, (m_pOption->ListBkColor >> 8) & 0xFF, m_pOption->ListBkColor & 0xFF));
	ListView_SetTextBkColor(m_hList, RGB((m_pOption->ListBkColor >> 16) & 0xFF, (m_pOption->ListBkColor >> 8) & 0xFF, m_pOption->ListBkColor & 0xFF));
}

void CListView::SetTextColor()
{
	//ListView_SetTextColor(m_hList, m_pOption->ListTextColor);
	ListView_SetTextColor(m_hList, RGB((m_pOption->ListTextColor >> 16) & 0xFF, (m_pOption->ListTextColor >> 8) & 0xFF, m_pOption->ListTextColor & 0xFF));
}

void CListView::Sort(int column)
{
	m_sort.column = column;
	m_sort.direct ^= 1;
	m_pSort = &m_sort;
	OnSort();
	InvalidateRect(m_hList, nullptr, FALSE);
}

void CListView::Enable(BOOL flag)
{
	EnableWindow(m_hList, flag);
}

void CListView::SetItemSelAll(UINT flag)
{
	ListView_SetItemState(m_hList, -1, flag, LVIS_SELECTED);
}

void CListView::SetFocus()
{
	::SetFocus(m_hList);
}

void CListView::SetWindowPos(int x, int y, int cx, int cy)
{
	MoveWindow(m_hList, x, y, cx, cy, TRUE);
}

void CListView::SaveIni()
{
	YCIni clIni(SBL_STR_INI_EXTRACTDATA);
	clIni.SetSection(m_uID);

	// Save column width
	int nColumns = Header_GetItemCount(ListView_GetHeader(m_hList));

	for (int i = 0 ; i < nColumns ; i++)
	{
		TCHAR szColumn[256];
		_stprintf(szColumn, _T("Column%d"), i);

		clIni.SetKey(szColumn);
		clIni.WriteDec(ListView_GetColumnWidth(m_hList, i));
	}
}

void CListView::Close()
{
	if (m_hImage)
	{
		ImageList_Destroy(m_hImage);
		m_hImage = nullptr;
	}
}

void CListView::Update()
{
	RECT rc;
	GetClientRect(m_hList, &rc);
	InvalidateRect(m_hList, &rc, FALSE);
}

UINT CListView::GetCountSel() const
{
	return ListView_GetSelectedCount(m_hList);
}

INT CListView::GetCount() const
{
	return ListView_GetItemCount(m_hList);
}

HWND CListView::GetHandle() const
{
	return m_hList;
}

int CListView::GetNextItem(int nItem) const
{
	return ListView_GetNextItem(m_hList, nItem, LVNI_ALL | LVNI_SELECTED);
}

int CListView::GetFocusItem() const
{
	return ListView_GetNextItem(m_hList, -1, LVNI_ALL | LVNI_FOCUSED);
}
