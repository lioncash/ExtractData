#pragma once

#include "Common.h"
#include "MainListView.h"
#include "MainToolBar.h"
#include "WindowBase.h"

class COption : public CWindowBase
{
public:
	void Init(CSearchToolBar& toolbar, CMainListView& listview);
	void LoadIni();
	static void SaveIni();

	void DoModal(HWND hWnd);
	HWND CreateProp(HWND hWnd);

	SOption& GetOpt() { return m_option; }
	HWND GetParent()  { return m_hParentWnd; }
	HWND GetHandle()  { return m_hDlg; }

private:
	static SOption m_option;
	static SOption m_option_tmp;
	static std::vector<YCString> m_SearchFiles;
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
