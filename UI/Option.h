#pragma once

#include "Common.h"
#include "UI/MainListView.h"
#include "UI/MainToolBar.h"
#include "UI/WindowBase.h"

class COption : public CWindowBase
{
public:
	void Init(CSearchToolBar& toolbar, CMainListView& listview);
	void LoadIni();
	static void SaveIni();

	void DoModal(HWND hWnd);
	HWND CreateProp(HWND hWnd);

	SOption& GetOpt();
	HWND GetParent() const;
	HWND GetHandle() const;

private:
	static SOption m_option;
	static SOption m_option_tmp;
	static CMainListView* m_pListView;
	static CSearchToolBar* m_pToolBar;
	static HWND m_hDlg;
	static HWND m_hParentWnd;

	static int CALLBACK PropSheetProc(HWND hWnd, UINT msg, LPARAM lParam);
	static LRESULT CALLBACK StdProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	static LRESULT CALLBACK ExtractProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	static LRESULT CALLBACK SusieProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	static void Apply();
};
