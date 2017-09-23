#include "StdAfx.h"
#include "UI/SusieListView.h"

#include "Common.h"
#include "res/ResExtractData.h"
#include "Susie.h"

void CSusieListView::Create(HWND hWnd, SOption& option, int x, int y, int cx, int cy)
{
	Init(hWnd, option);

	std::vector<LVCOLUMN> columns;
	LVCOLUMN column;

	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	column.fmt = LVCFMT_LEFT;
	column.cx = 102;
	column.pszText = _T("Plugin Name");
	columns.push_back(column);

	column.fmt = LVCFMT_LEFT;
	column.cx = 207;
	column.pszText = _T("Info");
	columns.push_back(column);

	column.fmt = LVCFMT_LEFT;
	column.cx = 62;
	column.pszText = _T("Support");
	columns.push_back(column);

	column.fmt = LVCFMT_LEFT;
	column.cx = 54;
	column.pszText = _T("Version");
	columns.push_back(column);

	// Create listview
	HWND list = CListView::Create(idsSusieList, std::move(columns), x, y, cx, cy);

	// Add checkbox to listview
	DWORD style = ListView_GetExtendedListViewStyle(list);
	style |= LVS_EX_CHECKBOXES;
	ListView_SetExtendedListViewStyle(list, style);

	ListView_SetCallbackMask(list, LVIS_STATEIMAGEMASK);
}

void CSusieListView::Show()
{
	CSusie susie;
	ListView_SetItemCountEx(m_list, susie.GetSusieTmp().size(), LVSICF_NOINVALIDATEALL);
}

void CSusieListView::Show(NMLVDISPINFO* disp_info)
{
	CSusie susie;

	static std::vector<SSusieInfo>& rvcSusieInfos = susie.GetSusieTmp();

	if (disp_info->item.mask & LVIF_TEXT)
	{
		switch (disp_info->item.iSubItem)
		{
		case 0: // Show plug-in name
			lstrcpy(disp_info->item.pszText, rvcSusieInfos[disp_info->item.iItem].name);
			break;

		case 1: // Show plug-in info
			lstrcpy(disp_info->item.pszText, rvcSusieInfos[disp_info->item.iItem].info);
			break;

		case 2: // Show supported formats
			lstrcpy(disp_info->item.pszText, rvcSusieInfos[disp_info->item.iItem].supported_formats);
			break;

		case 3: // Show version info
			lstrcpy(disp_info->item.pszText, rvcSusieInfos[disp_info->item.iItem].version);
			break;
		}
	}

	if (disp_info->item.mask & LVIF_STATE)
	{
		disp_info->item.state = m_option->bSusieUse ? INDEXTOSTATEIMAGEMASK(rvcSusieInfos[disp_info->item.iItem].validity + 1) : 0;
	}
}

void CSusieListView::ShowTip(LPNMLVGETINFOTIP tip)
{
	CSusie susie;

	static const std::vector<SSusieInfo>& susie_infos = susie.GetSusieTmp();

	switch (tip->iSubItem)
	{
	case 0:
		// dwFlags to display (when the character is hidden) in which case 0
		// Even when dwFlags is 1 (when the left-most column is not hidden)

		if (tip->dwFlags == 0)
		{
			lstrcpy(tip->pszText, susie_infos[tip->iItem].name);
		}
		break;

	case 1:
		lstrcpy(tip->pszText, susie_infos[tip->iItem].info);
		break;

	case 2:
		lstrcpy(tip->pszText, susie_infos[tip->iItem].supported_formats);
		break;

	case 3:
		lstrcpy(tip->pszText, susie_infos[tip->iItem].version);
		break;
	}
}

BOOL CSusieListView::CustomDraw(LPNMLVCUSTOMDRAW custom_draw)
{
	CSusie susie;

	static const std::vector<SSusieInfo>& susie_infos = susie.GetSusieTmp();

	switch (custom_draw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT: // Before drawing
		SetWindowLongPtr(m_window, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
		return TRUE;

	case CDDS_ITEMPREPAINT: // Item before it is drawn
		if (susie_infos[custom_draw->nmcd.dwItemSpec].has_config_dialog && m_option->bSusieUse)
		{
			// ConfigurationDlg has been defined
			custom_draw->clrText = RGB(0, 0, 255);
			SetWindowLongPtr(m_window, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);

			return TRUE;
		}
		break;
	}

	return FALSE;
}

void CSusieListView::CreateMenu(LPARAM param)
{
	int item = GetFocusItem();
	if (item == -1)
		return;

	CSusie susie;
	m_susie_info = susie.GetSusieTmp()[item];
	if (!m_susie_info.has_config_dialog)
		return;

	POINT pt;
	pt.x = LOWORD(param);
	pt.y = HIWORD(param);
	HMENU menu = LoadMenu(m_inst, _T("SUSIEMENU"));
	HMENU sub_menu = GetSubMenu(menu, 0);
	TrackPopupMenu(sub_menu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, m_window, nullptr);
	DestroyMenu(menu);
}

bool CSusieListView::SetCheck()
{
	// Get mouse cursor position on the list view
	LVHITTESTINFO info;
	GetCursorPos(&info.pt);
	ScreenToClient(m_list, &info.pt);

	// Get number of sub-item at the position of the mouse cursor
	info.flags = LVHT_ONITEMLABEL;
	ListView_SubItemHitTest(m_list, &info);

	// Make sure we do not get a non-zero or -1 (out of range) item number
	if (info.iItem == -1 || info.iSubItem != 0)
		return false;

	// Obtain the coordinates of the checkbox area
	RECT rc;
	ListView_GetSubItemRect(m_list, info.iItem, info.iSubItem, LVIR_BOUNDS, &rc);
	rc.left = 0;
	rc.right = 18;

	// Check processing is performed if the X coordinate of the mouse is within the checkbox region
	if (info.pt.x >= rc.left && info.pt.x <= rc.right)
	{
		CSusie susie;
		susie.GetSusieTmp()[info.iItem].validity ^= 1;
		InvalidateRect(m_list, &rc, FALSE);
		return true;
	}

	return false;
}

void CSusieListView::SetCheckAll(bool flag)
{
	CSusie susie;
	std::vector<SSusieInfo>& susie_infos = susie.GetSusieTmp();

	for (auto& info : susie_infos)
	{
		info.validity = flag;
	}

	RECT rc;
	GetClientRect(m_list, &rc);
	rc.left = 0, rc.right = 18;
	InvalidateRect(m_list, &rc, FALSE);
}
