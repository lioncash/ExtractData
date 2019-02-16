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
	column.pszText = const_cast<decltype(column.pszText)>(_T("No."));
	columns.push_back(column);

	column.fmt = LVCFMT_LEFT;
	column.cx = 215;
	column.pszText = const_cast<decltype(column.pszText)>(_T("File Name"));
	columns.push_back(column);

	column.fmt = LVCFMT_RIGHT;
	column.cx = 90;
	column.pszText = const_cast<decltype(column.pszText)>(_T("File Size"));
	columns.push_back(column);

	column.fmt = LVCFMT_RIGHT;
	column.cx = 105;
	column.pszText = const_cast<decltype(column.pszText)>(_T("Compressed Size"));
	columns.push_back(column);

	column.fmt = LVCFMT_RIGHT;
	column.cx = 75;
	column.pszText = const_cast<decltype(column.pszText)>(_T("File Format"));
	columns.push_back(column);

	column.fmt = LVCFMT_LEFT;
	column.cx = 85;
	column.pszText = const_cast<decltype(column.pszText)>(_T("Archive File"));
	columns.push_back(column);

	column.fmt = LVCFMT_RIGHT;
	column.cx = 100;
	column.pszText = const_cast<decltype(column.pszText)>(_T("Start Address"));
	columns.push_back(column);

	column.fmt = LVCFMT_RIGHT;
	column.cx = 100;
	column.pszText = const_cast<decltype(column.pszText)>(_T("End Address"));
	columns.push_back(column);

	HWND hList = CListView::Create(idsMainList, std::move(columns));
	SetBkColor();
	SetTextColor();
}

void CMainListView::Show()
{
	ListView_SetItemCountEx(m_list, m_ent.size(), LVSICF_NOINVALIDATEALL);
}

void CMainListView::Show(NMLVDISPINFO* disp_info)
{
	if (disp_info->item.mask & LVIF_TEXT)
	{
		const SFileInfo& file_info = m_ent[disp_info->item.iItem];
		const int        text_max = disp_info->item.cchTextMax - 1;

		switch(disp_info->item.iSubItem)
		{
		case 0: // No view
			_stprintf(disp_info->item.pszText, _T("%6d."), (disp_info->item.iItem + 1));
			break;

		case 1: // Show file name
			lstrcpy(disp_info->item.pszText, file_info.name.Left(text_max));
			break;

		case 2: // Show file size
			lstrcpy(disp_info->item.pszText, file_info.size_org_comma.Left(text_max));
			break;

		case 3: // Show compressed file size
			if (file_info.size_cmp != file_info.size_org)
			{
				lstrcpy(disp_info->item.pszText, file_info.size_cmp_comma.Left(text_max));
			}
			break;

		case 4: // Show file format
			lstrcpy(disp_info->item.pszText, file_info.format.Left(text_max));
			break;

		case 5: // Display the archive filename
			lstrcpy(disp_info->item.pszText, file_info.arc_name.Left(text_max));
			break;

		case 6: // Display the start address
			_stprintf(disp_info->item.pszText, _T("0x%llx"), file_info.start);
			break;

		case 7: // Display the end address
			_stprintf(disp_info->item.pszText, _T("0x%llx"), file_info.end);
			break;
		}
	}
}

void CMainListView::ShowTip(LPNMLVGETINFOTIP tip)
{
	switch (tip->iSubItem)
	{
	case 0:
		// dwFlags to display (when the character is hidden), in this case 0
		// dwFlags (even when the left-most column is not hidden) in this case 1
		if (tip->dwFlags == 0)
			_stprintf(tip->pszText, _T("%6d"), tip->iItem + 1);
		break;
	case 1:
		lstrcpy(tip->pszText, m_ent[tip->iItem].name);
		break;
	case 2:
		lstrcpy(tip->pszText, m_ent[tip->iItem].size_org_comma);
		break;
	case 3:
		if (m_ent[tip->iItem].size_cmp != m_ent[tip->iItem].size_org)
			lstrcpy(tip->pszText, m_ent[tip->iItem].size_cmp_comma);
		break;
	case 4:
		lstrcpy(tip->pszText, m_ent[tip->iItem].format);
		break;
	case 5:
		lstrcpy(tip->pszText, m_ent[tip->iItem].arc_name);
		break;
	case 6:
		_stprintf(tip->pszText, _T("0x%llx"), m_ent[tip->iItem].start);
		break;
	case 7:
		_stprintf(tip->pszText, _T("0x%llx"), m_ent[tip->iItem].end);
		break;
	}
}

// Comparison function for sorting
bool CMainListView::CompareFunc(const SFileInfo& a, const SFileInfo& b)
{
	switch (m_sort_ptr->column)
	{
	case 1:
		return retCompare(a.name, b.name);
	case 2:
		return retCompare(a.size_org, b.size_org);
	case 3:
		return retCompare(a.size_cmp, b.size_cmp);
	case 4:
		return retCompare(a.format, b.format);
	case 5:
		return retCompare(a.arc_name, b.arc_name);
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
	for (auto& file_info : m_ent)
	{
		if (!file_info.sizes_org.empty())
		{
			file_info.sizes_org.clear();
			file_info.sizes_cmp.clear();
			file_info.starts.clear();
			file_info.compress_checks.clear();
		}
		file_info.tmp_file_paths.clear();
	}
	m_ent.clear();

	// Erase all the file information being displayed
	ListView_DeleteAllItems(m_list);
	ListView_SetItemCountEx(m_list, 0, LVSICF_NOINVALIDATEALL);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Drag has been initiated

void CMainListView::OnBeginDrag(NMHDR* nmhdr, LRESULT* result)
{
	NM_LISTVIEW* pNMListView = reinterpret_cast<NM_LISTVIEW*>(nmhdr);
}
