#include "StdAfx.h"
#include "UI/MainToolBar.h"

#include "ArcFile.h"
#include "Common.h"
#include "res/ResExtractData.h"
#include "UI/Option.h"

///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                            Main toolbar                               //
//                                                                       //
///////////////////////////////////////////////////////////////////////////
HWND CMainToolBar::Create(HWND hWnd)
{
	std::array<TBBUTTON, 3> buttons{{
		{0, IDM_OPEN, TBSTATE_ENABLED, TBSTYLE_DROPDOWN, 0, 0},
		{1, IDM_EXTRACT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
		{2, IDM_EXTRACTALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0}
	}};

	HWND toolbar = CToolBar::Create(hWnd, buttons.data(), IDI_MAIN_TOOLBAR, 16, buttons.size());

	TBBUTTON space = {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0};
	SendMessage(toolbar, TB_INSERTBUTTON, 0, reinterpret_cast<LPARAM>(&space));
	SendMessage(toolbar, TB_INSERTBUTTON, 4, reinterpret_cast<LPARAM>(&space));

	return toolbar;
}

void CMainToolBar::CreateMenuHistory(int item)
{
	RECT rc;
	SendMessage(GetCtrlHandle(), TB_GETRECT, static_cast<WPARAM>(item), reinterpret_cast<LPARAM>(&rc));
	MapWindowPoints(GetCtrlHandle(), HWND_DESKTOP, reinterpret_cast<LPPOINT>(&rc), 2);

	TPMPARAMS tpm;
	tpm.cbSize = sizeof(TPMPARAMS);
	tpm.rcExclude.top = rc.top;
	tpm.rcExclude.left = rc.left;
	tpm.rcExclude.bottom = rc.bottom;
	tpm.rcExclude.right = rc.right;

	HMENU open_menu = CreateMenu();
	HMENU open_sub_menu = CreatePopupMenu();

	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_TYPE | MIIM_SUBMENU;
	mii.fType = MFT_STRING;
	mii.hSubMenu = open_sub_menu;
	mii.dwTypeData = _T("dummy");
	InsertMenuItem(open_menu, 0, TRUE, &mii);

	// When there is no history
	if (m_open_history_list.empty())
	{
		mii.fMask = MIIM_TYPE | MIIM_STATE;
		mii.dwTypeData = _T("No History");
		mii.fState = MFS_DISABLED;
		InsertMenuItem(open_sub_menu, 0, TRUE, &mii);
	}
	// When there is history
	else
	{
		mii.fMask = MIIM_TYPE | MIIM_ID;

		for (int i = 0; i < static_cast<int>(m_open_history_list.size()); i++)
		{
			mii.wID = ID_TOOLBAR_OPEN_HISTORY + i;
			mii.dwTypeData = m_open_history_list[i].GetBuffer(0);
			InsertMenuItem(open_sub_menu, i, TRUE, &mii);
		}
	}

	TrackPopupMenuEx(open_sub_menu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, GetHandle(), &tpm);
	DestroyMenu(open_menu);
}

void CMainToolBar::AddOpenHistory(std::vector<std::unique_ptr<CArcFile>>& archive_list)
{
	for (auto arc_iter = archive_list.begin(); arc_iter != archive_list.end(); )
	{
		const CArcFile* archive = arc_iter->get();
		// Turn off if there is any history
		for (auto iter_str = m_open_history_list.begin(); iter_str != m_open_history_list.end(); ++iter_str)
		{
			if (archive->GetArcPath() == *iter_str)
			{
				m_open_history_list.erase(iter_str);
				break;
			}
		}
		// When history entries have hit their limit, remove the oldest entry to make room for the newer one
		if (m_open_history_list.size() == 10)
			m_open_history_list.pop_back();
		// Add to the history
		m_open_history_list.insert(m_open_history_list.begin(), archive->GetArcPath());

		// Close a file that isn't supported
		if (!archive->GetState())
		{
			arc_iter = archive_list.erase(arc_iter);
		}
		else
		{
			++arc_iter;
		}
	}

	// Save the history ini
	SaveIni();
}

void CMainToolBar::LoadIni()
{
	YCIni ini(SBL_STR_INI_EXTRACTDATA);
	ini.SetSection(_T("OpenHistory"));

	for (unsigned int i = 0; i < 10; i++)
	{
		TCHAR key[256];
		_stprintf(key, _T("File%u"), i);
		ini.SetKey(key);

		YCString archive_path;
		ini.ReadStr(archive_path, _T(""));

		if (archive_path == _T(""))
			break;

		m_open_history_list.push_back(archive_path);
	}
}

void CMainToolBar::SaveIni()
{
	YCIni ini(SBL_STR_INI_EXTRACTDATA);
	ini.SetSection(_T("OpenHistory"));

	for (size_t i = 0; i < m_open_history_list.size(); i++)
	{
		TCHAR key[256];
		_stprintf(key, _T("File%zu"), i);
		ini.SetKey(key);
		ini.WriteStr(m_open_history_list[i]);
	}
}

void CMainToolBar::SetWindowPos(int x, int y, int cx, int cy)
{
	MoveWindow(GetCtrlHandle(), x, y, cx, cy, TRUE);
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                         Search file toolbar                           //
//                                                                       //
///////////////////////////////////////////////////////////////////////////
HWND CSearchToolBar::Create(HWND hWnd)
{
	std::array<TBBUTTON, 9> buttons{{
		{0, IDM_AHX, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
		{1, IDM_BMP, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
		{2, IDM_JPG, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
		{3, IDM_MID, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
		{4, IDM_MPG, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
		{5, IDM_OGG, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
		{6, IDM_PNG, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
		{7, IDM_WAV, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
		{8, IDM_WMV, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0}
	}};

	m_begin_id = IDM_AHX;

	HWND toolbar = CToolBar::Create(hWnd, buttons.data(), IDI_SEARCH_TOOLBAR, 25, buttons.size());
	SetCheckSearch();

	return toolbar;
}

void CSearchToolBar::SetCheckSearch()
{
	HWND toolbar = GetCtrlHandle();
	COption option;

	for (size_t i = 0, ID = m_begin_id; i < option.GetOpt().bSearch.size(); i++, ID++)
	{
		SendMessage(toolbar, TB_CHECKBUTTON, ID, option.GetOpt().bSearch[i]);
	}
}

void CSearchToolBar::SetSearch(int id)
{
	const int number = id - m_begin_id;

	// Change
	COption option;
	option.GetOpt().bSearch[number] ^= 1;

	// Save ini
	option.SaveIni();
}

void CSearchToolBar::SetWindowPos(int x, int y, int cx, int cy)
{
	MoveWindow(GetCtrlHandle(), x, y, cx, cy, TRUE);
}
