#include "StdAfx.h"
#include "UI/SusieListView.h"

#include "Common.h"
#include "res/ResExtractData.h"
#include "Susie.h"

void CSusieListView::Create(HWND hWnd, SOption& option, int x, int y, int cx, int cy)
{
	Init(hWnd, option);

	std::vector<LVCOLUMN> lvcols;
	LVCOLUMN lvcol;

	lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvcol.fmt = LVCFMT_LEFT;
	lvcol.cx = 102;
	lvcol.pszText = _T("Plugin Name");
	lvcols.push_back(lvcol);

	lvcol.fmt = LVCFMT_LEFT;
	lvcol.cx = 207;
	lvcol.pszText = _T("Info");
	lvcols.push_back(lvcol);

	lvcol.fmt = LVCFMT_LEFT;
	lvcol.cx = 62;
	lvcol.pszText = _T("Support");
	lvcols.push_back(lvcol);

	lvcol.fmt = LVCFMT_LEFT;
	lvcol.cx = 54;
	lvcol.pszText = _T("Version");
	lvcols.push_back(lvcol);

	// Create listview
	HWND hList = CListView::Create(idsSusieList, lvcols, x, y, cx, cy);

	// Add checkbox to listview
	DWORD dwStyle = ListView_GetExtendedListViewStyle(hList);
	dwStyle |= LVS_EX_CHECKBOXES;
	ListView_SetExtendedListViewStyle(hList, dwStyle);

	ListView_SetCallbackMask(hList, LVIS_STATEIMAGEMASK);
}

void CSusieListView::Show()
{
	CSusie susie;
	ListView_SetItemCountEx(m_hList, susie.GetSusieTmp().size(), LVSICF_NOINVALIDATEALL);
}

void CSusieListView::Show(NMLVDISPINFO* pDispInfo)
{
	CSusie clSusie;

	static std::vector<SSusieInfo>& rvcSusieInfos = clSusie.GetSusieTmp();

	if (pDispInfo->item.mask & LVIF_TEXT)
	{
		switch (pDispInfo->item.iSubItem)
		{
		case 0: // Show plug-in name
			lstrcpy(pDispInfo->item.pszText, rvcSusieInfos[pDispInfo->item.iItem].clsName);
			break;

		case 1: // Show plug-in info
			lstrcpy(pDispInfo->item.pszText, rvcSusieInfos[pDispInfo->item.iItem].clsInfo);
			break;

		case 2: // Show supported formats
			lstrcpy(pDispInfo->item.pszText, rvcSusieInfos[pDispInfo->item.iItem].clsSupportFormat);
			break;

		case 3: // Show version info
			lstrcpy(pDispInfo->item.pszText, rvcSusieInfos[pDispInfo->item.iItem].clsVersion);
			break;
		}
	}

	if( pDispInfo->item.mask & LVIF_STATE )
	{
		pDispInfo->item.state = m_pOption->bSusieUse ? INDEXTOSTATEIMAGEMASK(rvcSusieInfos[pDispInfo->item.iItem].bValidity + 1) : 0;
	}
}

void CSusieListView::ShowTip(LPNMLVGETINFOTIP ptip)
{
	CSusie clSusie;

	static std::vector<SSusieInfo>&	rvcSusieInfos = clSusie.GetSusieTmp();

	switch (ptip->iSubItem)
	{
	case 0:
		// dwFlags to display (when the character is hidden) in which case 0
		// Even when dwFlags is 1 (when the left-most column is not hidden)

		if (ptip->dwFlags == 0)
		{
			lstrcpy(ptip->pszText, rvcSusieInfos[ptip->iItem].clsName);
		}
		break;

	case 1:
		lstrcpy(ptip->pszText, rvcSusieInfos[ptip->iItem].clsInfo);
		break;

	case 2:
		lstrcpy(ptip->pszText, rvcSusieInfos[ptip->iItem].clsSupportFormat);
		break;

	case 3:
		lstrcpy(ptip->pszText, rvcSusieInfos[ptip->iItem].clsVersion);
		break;
	}
}

BOOL CSusieListView::CustomDraw(LPNMLVCUSTOMDRAW plvcd)
{
	CSusie clSusie;

	static std::vector<SSusieInfo>&	rvcSusieInfos = clSusie.GetSusieTmp();
	static SOption* pOption = m_pOption;

	switch (plvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT: // Before drawing
		SetWindowLongPtr(m_hWnd, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
		return TRUE;

	case CDDS_ITEMPREPAINT: // Item before it is drawn
		if (rvcSusieInfos[plvcd->nmcd.dwItemSpec].bConfig && pOption->bSusieUse)
		{
			// ConfigurationDlg has been defined
			plvcd->clrText = RGB(0, 0, 255);
			SetWindowLongPtr(m_hWnd, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);

			return TRUE;
		}
		break;
	}

	return FALSE;
}

void CSusieListView::CreateMenu(LPARAM lp)
{
	int nItem = GetFocusItem();
	if (nItem == -1)
		return;

	CSusie clSusie;
	m_SusieInfo = clSusie.GetSusieTmp()[nItem];
	if (!m_SusieInfo.bConfig)
		return;

	POINT pt;
	pt.x = LOWORD(lp);
	pt.y = HIWORD(lp);
	HMENU rMenu = LoadMenu(m_hInst, _T("SUSIEMENU"));
	HMENU rSubMenu = GetSubMenu(rMenu, 0);
	TrackPopupMenu(rSubMenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, m_hWnd, nullptr);
	DestroyMenu(rMenu);
}

bool CSusieListView::SetCheck()
{
	// Get mouse cursor position on the list view
	LVHITTESTINFO htInfo;
	GetCursorPos(&htInfo.pt);
	ScreenToClient(m_hList, &htInfo.pt);

	// Get number of sub-item at the position of the mouse cursor
	htInfo.flags = LVHT_ONITEMLABEL;
	ListView_SubItemHitTest(m_hList, &htInfo);

	// Make sure we do not get a non-zero or -1 (out of range) item number
	if (htInfo.iItem == -1 || htInfo.iSubItem != 0)
		return false;

	// Obtain the coordinates of the checkbox area
	RECT rc;
	ListView_GetSubItemRect(m_hList, htInfo.iItem, htInfo.iSubItem, LVIR_BOUNDS, &rc);
	rc.left = 0;
	rc.right = 18;

	// Check processing is performed if the X coordinate of the mouse is within the checkbox region
	if (htInfo.pt.x >= rc.left && htInfo.pt.x <= rc.right)
	{
		CSusie susie;
		susie.GetSusieTmp()[htInfo.iItem].bValidity ^= 1;
		InvalidateRect(m_hList, &rc, FALSE);
		return true;
	}

	return false;
}

void CSusieListView::SetCheckAll(bool flag)
{
	CSusie susie;
	std::vector<SSusieInfo>& SusieInfos = susie.GetSusieTmp();

	for (auto& info : SusieInfos)
	{
		info.bValidity = flag;
	}

	RECT rc;
	GetClientRect(m_hList, &rc);
	rc.left = 0, rc.right = 18;
	InvalidateRect(m_hList, &rc, FALSE);
}
