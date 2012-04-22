#pragma once

#include "Ctrl/ToolBar.h"

#define ID_TOOLBAR_OPEN_HISTORY 20000

// Main toolbar
class CMainToolBar : public CToolBar {
private:
	std::vector<YCString> m_vcOpenHistoryList;

public:
	HWND Create(HWND hWnd);
	void CreateMenuHistory(int iItem);
	void AddOpenHistory(std::vector<CArcFile*>& pclArcList);
	std::vector<YCString>& GetHistory() { return (m_vcOpenHistoryList); }

	void LoadIni();
	void SaveIni();

	void SetWindowPos(int x, int y, int cx, int cy) { MoveWindow(GetCtrlHandle(), x, y, cx, cy, TRUE); }
};

// Search file toolbar
class CSearchToolBar : public CToolBar {
private:
	int m_nBeginID;

public:
	HWND Create(HWND hWnd);
	void SetCheckSearch();
	void SetSearch(int nID);
	void SetWindowPos(int x, int y, int cx, int cy) { MoveWindow(GetCtrlHandle(), x, y, cx, cy, TRUE); }
};