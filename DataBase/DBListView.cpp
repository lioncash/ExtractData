#include "stdafx.h"
#include "../res/ResExtractData.h"
#include "DataBase.h"
#include "DBListView.h"

// A method that sets the values of the database list
void CDBListView::Create(HWND hWnd, OPTION& option)
{
	Init(hWnd, option);

	std::vector<LVCOLUMN> lvcols;
	LVCOLUMN lvcol;

	lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvcol.fmt = LVCFMT_LEFT;
	lvcol.cx = 45;
	lvcol.pszText = _T("No.");
	lvcols.push_back(lvcol);

	lvcol.fmt = LVCFMT_LEFT;
	lvcol.cx = 305;
	lvcol.pszText = _T("Path");
	lvcols.push_back(lvcol);

	lvcol.fmt = LVCFMT_RIGHT;
	lvcol.cx = 90;
	lvcol.pszText = _T("File Count");
	lvcols.push_back(lvcol);

	CListView::Create(idsDataBaseList, lvcols);
}

void CDBListView::Show(NMLVDISPINFO* pDispInfo)
{
	switch (pDispInfo->item.iSubItem)
	{
		case 0: // No. View
			wsprintf(pDispInfo->item.pszText, _T("%6d."), pDispInfo->item.iItem + 1);
			break;

		case 1: // Show path
			_tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, m_entDB[pDispInfo->item.iItem].path.c_str());
			break;

		case 2: // Show file info
			_tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, m_entDB[pDispInfo->item.iItem].sCtFile.c_str());
			break;
	}
}

// Comparison function for sorting
BOOL CDBListView::CompareFunc(const FILEINFODB& a, const FILEINFODB& b)
{
	static SORTPARAM* pSort = m_s_pSort;

	switch (pSort->column)
	{
		case 1:
			return (retCompare(a.path, b.path));

		case 2:
			return (retCompare(a.ctFile, b.ctFile));
	}

	return FALSE;
}

void CDBListView::Sort()
{
	// Copy the static member variable
	m_s_pSort = &m_sort;

	// Sort
	std::sort(m_entDB.begin(), m_entDB.end(), CompareFunc);
	InvalidateRect(m_hList, NULL, FALSE);
}

// Function to sort a list of databases
void CDBListView::Sort(int column)
{
	// Copy the static member variable
	m_s_pSort = &m_sort;

	// Reverse the column if is clicked upon twice
	if ((column == 0) || (m_sort.column == column))
	{
		m_sort.direct ^= 1;
		std::reverse(m_entDB.begin(), m_entDB.end());
	}
	else
	{
		m_sort.direct = 0;
		std::sort(m_entDB.begin(), m_entDB.end(), CompareFunc);
	}
	
	InvalidateRect(m_hList, NULL, FALSE);
}

// Function to redraw the database list
void CDBListView::Update(int StartItem)
{
	// Update number of items
	ListView_SetItemCountEx(m_hList, m_entDB.size(), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);

	// Will not be drawn with the selected state and will become zero. Redraw properly
	if (m_entDB.empty())
	{
		ListView_Update(m_hList, 0);
	}
	else
	{
		int nCountPerPage = ListView_GetCountPerPage(m_hList);
		int nVisibleLast = StartItem + nCountPerPage;

		// Redraw the listview (which is to redraw the selected state. This is a fairly aggressive technique)
		for (int i = StartItem; i <= nVisibleLast; i++)
			ListView_SetItemState(m_hList, i, LVIS_SELECTED, LVIS_SELECTED);

		ListView_SetItemState(m_hList, -1, 0, LVIS_SELECTED);
	}
}
