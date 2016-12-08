#include "StdAfx.h"
#include "UI/MainListView.h"

#include "res/ResExtractData.h"

// A method that sets the values of the main list
void CMainListView::Create(HWND hWnd, SOption& option)
{
	Init(hWnd, option);

	std::vector<LVCOLUMN> columns;
	LVCOLUMN column;

	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	column.fmt = LVCFMT_LEFT;
	column.cx = 45;
	column.pszText = _T("No.");
	columns.push_back(column);

	column.fmt = LVCFMT_LEFT;
	column.cx = 215;
	column.pszText = _T("File Name");
	columns.push_back(column);

	column.fmt = LVCFMT_RIGHT;
	column.cx = 90;
	column.pszText = _T("File Size");
	columns.push_back(column);

	column.fmt = LVCFMT_RIGHT;
	column.cx = 105;
	column.pszText = _T("Compressed Size");
	columns.push_back(column);

	column.fmt = LVCFMT_RIGHT;
	column.cx = 75;
	column.pszText = _T("File Format");
	columns.push_back(column);

	column.fmt = LVCFMT_LEFT;
	column.cx = 85;
	column.pszText = _T("Archive File");
	columns.push_back(column);

	column.fmt = LVCFMT_RIGHT;
	column.cx = 100;
	column.pszText = _T("Start Address");
	columns.push_back(column);

	column.fmt = LVCFMT_RIGHT;
	column.cx = 100;
	column.pszText = _T("End Address");
	columns.push_back(column);

	HWND hList = CListView::Create(idsMainList, std::move(columns));
	SetBkColor();
	SetTextColor();
}

void CMainListView::Show()
{
	ListView_SetItemCountEx(m_hList, m_ent.size(), LVSICF_NOINVALIDATEALL);
}

void CMainListView::Show(NMLVDISPINFO* pstDispInfo)
{
	if (pstDispInfo->item.mask & LVIF_TEXT)
	{
		const SFileInfo& file_info = m_ent[pstDispInfo->item.iItem];
		const int        text_max = pstDispInfo->item.cchTextMax - 1;

		switch(pstDispInfo->item.iSubItem)
		{
		case 0: // No view
			_stprintf(pstDispInfo->item.pszText, _T("%6d."), (pstDispInfo->item.iItem + 1));
			break;

		case 1: // Show file name
			//_tcscpy_s(pstDispInfo->item.pszText, pstDispInfo->item.cchTextMax-100, pEnt[pstDispInfo->item.iItem].name);
			lstrcpy(pstDispInfo->item.pszText, file_info.name.Left(text_max));
			//pstDispInfo->item.pszText = rfEnt[pstDispInfo->item.iItem].name.GetBuffer(0);
			break;

		case 2: // Show file size
			lstrcpy(pstDispInfo->item.pszText, file_info.sSizeOrg.Left(text_max));
			//pstDispInfo->item.pszText = rfEnt[pstDispInfo->item.iItem].sSizeOrg.GetBuffer(0);
			break;

		case 3: // Show compressed file size
			if (file_info.sizeCmp != file_info.sizeOrg)
			{
				//pstDispInfo->item.pszText = rfEnt[pstDispInfo->item.iItem].sSizeCmp.GetBuffer(0);
				lstrcpy(pstDispInfo->item.pszText, file_info.sSizeCmp.Left(text_max));
			}
			break;

		case 4: // Show file format
			lstrcpy(pstDispInfo->item.pszText, file_info.format.Left(text_max));
			//pstDispInfo->item.pszText = rfEnt[pstDispInfo->item.iItem].format.GetBuffer(0);
			break;

		case 5: // Display the archive filename
			lstrcpy(pstDispInfo->item.pszText, file_info.arcName.Left(text_max));
			//pstDispInfo->item.pszText = rfEnt[pstDispInfo->item.iItem].arcName.GetBuffer(0);
			break;

		case 6: // Display the start address
			_stprintf(pstDispInfo->item.pszText, _T("0x%llx"), file_info.start);
			break;

		case 7: // Display the end address
			_stprintf(pstDispInfo->item.pszText, _T("0x%llx"), file_info.end);
			break;
		}
	}
}

void CMainListView::ShowTip(LPNMLVGETINFOTIP ptip)
{
	switch (ptip->iSubItem)
	{
	case 0:
		// dwFlags to display (when the character is hidden), in this case 0
		// dwFlags (even when the left-most column is not hidden) in this case 1
		if (ptip->dwFlags == 0)
			_stprintf(ptip->pszText, _T("%6d"), ptip->iItem + 1);
		break;
	case 1:
		lstrcpy(ptip->pszText, m_ent[ptip->iItem].name);
		break;
	case 2:
		lstrcpy(ptip->pszText, m_ent[ptip->iItem].sSizeOrg);
		break;
	case 3:
		if (m_ent[ptip->iItem].sizeCmp != m_ent[ptip->iItem].sizeOrg)
			lstrcpy(ptip->pszText, m_ent[ptip->iItem].sSizeCmp);
		break;
	case 4:
		lstrcpy(ptip->pszText, m_ent[ptip->iItem].format);
		break;
	case 5:
		lstrcpy(ptip->pszText, m_ent[ptip->iItem].arcName);
		break;
	case 6:
		_stprintf(ptip->pszText, _T("0x%llx"), m_ent[ptip->iItem].start);
		break;
	case 7:
		_stprintf(ptip->pszText, _T("0x%llx"), m_ent[ptip->iItem].end);
		break;
	}
}

// Comparison function for sorting
bool CMainListView::CompareFunc(const SFileInfo& a, const SFileInfo& b)
{
	switch (m_pSort->column)
	{
	case 1:
		return retCompare(a.name, b.name);
	case 2:
		return retCompare(a.sizeOrg, b.sizeOrg);
	case 3:
		return retCompare(a.sizeCmp, b.sizeCmp);
	case 4:
		return retCompare(a.format, b.format);
	case 5:
		return retCompare(a.arcName, b.arcName);
	case 6:
		return retCompare(a.start, b.start);
	case 7:
		return retCompare(a.end, b.end);
	}

	return false;
}

// Function to sort a listview
void CMainListView::OnSort()
{
	// Note: Reverse when clicked
	if (m_sort.column == 0)
		std::reverse(m_ent.begin(), m_ent.end());
	else
		std::sort(m_ent.begin(), m_ent.end(), CompareFunc);
}

void CMainListView::Clear()
{
	if (m_ent.empty())
		return;

	// Delete files
	for (auto& fileInfo : m_ent)
	{
		if (!fileInfo.sizesOrg.empty())
		{
			fileInfo.sizesOrg.clear();
			fileInfo.sizesCmp.clear();
			fileInfo.starts.clear();
			fileInfo.bCmps.clear();
		}
		fileInfo.sTmpFilePath.clear();
	}
	m_ent.clear();

	// Erase all the file information being displayed
	ListView_DeleteAllItems(m_hList);
	ListView_SetItemCountEx(m_hList, 0, LVSICF_NOINVALIDATEALL);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Drag has been initiated

void CMainListView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
}
