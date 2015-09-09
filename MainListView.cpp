#include "stdafx.h"
#include "res/ResExtractData.h"
#include "MainListView.h"

// A method that sets the values of the main list
void CMainListView::Create(HWND hWnd, SOption& option)
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
	lvcol.cx = 215;
	lvcol.pszText = _T("File Name");
	lvcols.push_back(lvcol);

	lvcol.fmt = LVCFMT_RIGHT;
	lvcol.cx = 90;
	lvcol.pszText = _T("File Size");
	lvcols.push_back(lvcol);

	lvcol.fmt = LVCFMT_RIGHT;
	lvcol.cx = 105;
	lvcol.pszText = _T("Compressed Size");
	lvcols.push_back(lvcol);

	lvcol.fmt = LVCFMT_RIGHT;
	lvcol.cx = 75;
	lvcol.pszText = _T("File Format");
	lvcols.push_back(lvcol);

	lvcol.fmt = LVCFMT_LEFT;
	lvcol.cx = 85;
	lvcol.pszText = _T("Archive File");
	lvcols.push_back(lvcol);

	lvcol.fmt = LVCFMT_RIGHT;
	lvcol.cx = 100;
	lvcol.pszText = _T("Start Address");
	lvcols.push_back(lvcol);

	lvcol.fmt = LVCFMT_RIGHT;
	lvcol.cx = 100;
	lvcol.pszText = _T("End Address");
	lvcols.push_back(lvcol);

	HWND hList = CListView::Create(idsMainList, lvcols);
	SetBkColor();
	SetTextColor();
}

void CMainListView::Show()
{
	ListView_SetItemCountEx(m_hList, m_ent.size(), LVSICF_NOINVALIDATEALL);
}

void CMainListView::Show(NMLVDISPINFO* pstDispInfo)
{
	static std::vector<SFileInfo>& rfEnt = m_ent;

	if (pstDispInfo->item.mask & LVIF_TEXT)
	{
		SFileInfo& rfstFileInfo = rfEnt[pstDispInfo->item.iItem];
		int        nTextMax = (pstDispInfo->item.cchTextMax - 1 );

		switch(pstDispInfo->item.iSubItem)
		{
		case 0: // No view
			_stprintf(pstDispInfo->item.pszText, _T("%6d."), (pstDispInfo->item.iItem + 1));
			break;

		case 1: // Show file name
			//_tcscpy_s(pstDispInfo->item.pszText, pstDispInfo->item.cchTextMax-100, pEnt[pstDispInfo->item.iItem].name);
			lstrcpy(pstDispInfo->item.pszText, rfstFileInfo.name.Left(nTextMax));
			//pstDispInfo->item.pszText = rfEnt[pstDispInfo->item.iItem].name.GetBuffer(0);
			break;

		case 2: // Show file size
			lstrcpy(pstDispInfo->item.pszText, rfstFileInfo.sSizeOrg.Left(nTextMax));
			//pstDispInfo->item.pszText = rfEnt[pstDispInfo->item.iItem].sSizeOrg.GetBuffer(0);
			break;

		case 3: // Show compressed file size
			if (rfstFileInfo.sizeCmp != rfstFileInfo.sizeOrg)
			{
				//pstDispInfo->item.pszText = rfEnt[pstDispInfo->item.iItem].sSizeCmp.GetBuffer(0);
				lstrcpy(pstDispInfo->item.pszText, rfstFileInfo.sSizeCmp.Left(nTextMax));
			}
			break;

		case 4: // Show file format
			lstrcpy(pstDispInfo->item.pszText, rfstFileInfo.format.Left(nTextMax));
			//pstDispInfo->item.pszText = rfEnt[pstDispInfo->item.iItem].format.GetBuffer(0);
			break;

		case 5: // Display the archive filename
			lstrcpy(pstDispInfo->item.pszText, rfstFileInfo.arcName.Left(nTextMax));
			//pstDispInfo->item.pszText = rfEnt[pstDispInfo->item.iItem].arcName.GetBuffer(0);
			break;

		case 6: // Display the start address
			_stprintf(pstDispInfo->item.pszText, _T("0x%llx"), rfstFileInfo.start);
			break;

		case 7: // Display the end address
			_stprintf(pstDispInfo->item.pszText, _T("0x%llx"), rfstFileInfo.end);
			break;
		}
	}
}

void CMainListView::ShowTip(LPNMLVGETINFOTIP ptip)
{
	static std::vector<SFileInfo>& pEnt = m_ent;

	switch (ptip->iSubItem)
	{
		case 0:
			// dwFlags to display (when the character is hidden), in this case 0
			// dwFlags (even when the left-most column is not hidden) in this case 1
			if (ptip->dwFlags == 0)
				_stprintf(ptip->pszText, _T("%6d"), ptip->iItem + 1);
			break;
		case 1:
			lstrcpy(ptip->pszText, pEnt[ptip->iItem].name);
			break;
		case 2:
			lstrcpy(ptip->pszText, pEnt[ptip->iItem].sSizeOrg);
			break;
		case 3:
			if (pEnt[ptip->iItem].sizeCmp != pEnt[ptip->iItem].sizeOrg)
				lstrcpy(ptip->pszText, pEnt[ptip->iItem].sSizeCmp);
			break;
		case 4:
			lstrcpy(ptip->pszText, pEnt[ptip->iItem].format);
			break;
		case 5:
			lstrcpy(ptip->pszText, pEnt[ptip->iItem].arcName);
			break;
		case 6:
			_stprintf(ptip->pszText, _T("0x%llx"), pEnt[ptip->iItem].start);
			break;
		case 7:
			_stprintf(ptip->pszText, _T("0x%llx"), pEnt[ptip->iItem].end);
			break;
	}
}

// Comparison function for sorting
BOOL CMainListView::CompareFunc(const SFileInfo& a, const SFileInfo& b)
{
	static SORTPARAM* pSort = m_pSort;
	switch (pSort->column)
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

	return FALSE;
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
	std::vector<SFileInfo>& pEnt = m_ent;
	if (!pEnt.empty())
	{
		// Delete files
		for (size_t i = 0; i < pEnt.size(); i++)
		{
			if (!pEnt[i].sizesOrg.empty())
			{
				pEnt[i].sizesOrg.clear();
				pEnt[i].sizesCmp.clear();
				pEnt[i].starts.clear();
				pEnt[i].bCmps.clear();
			}
			pEnt[i].sTmpFilePath.clear();
		}
		pEnt.clear();

		// Erase all the file information being displayed
		ListView_DeleteAllItems(m_hList);
		ListView_SetItemCountEx(m_hList, 0, LVSICF_NOINVALIDATEALL);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Drag has been initiated

void CMainListView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
}
