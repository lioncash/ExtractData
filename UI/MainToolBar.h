#pragma once

#include "UI/Ctrl/ToolBar.h"

class CArcFile;

#define ID_TOOLBAR_OPEN_HISTORY 20000

// Main toolbar
class CMainToolBar : public CToolBar
{
public:
	HWND Create(HWND hWnd);
	void CreateMenuHistory(int iItem);
	void AddOpenHistory(std::vector<std::unique_ptr<CArcFile>>& archive_list);
	const std::vector<YCString>& GetHistory() const { return m_vcOpenHistoryList; }

	void LoadIni() override;
	void SaveIni() override;

	void SetWindowPos(int x, int y, int cx, int cy) { MoveWindow(GetCtrlHandle(), x, y, cx, cy, TRUE); }

private:
	std::vector<YCString> m_vcOpenHistoryList;
};

// Search file toolbar
class CSearchToolBar : public CToolBar
{
public:
	HWND Create(HWND hWnd);
	void SetCheckSearch();
	void SetSearch(int nID);
	void SetWindowPos(int x, int y, int cx, int cy) { MoveWindow(GetCtrlHandle(), x, y, cx, cy, TRUE); }

private:
	int m_nBeginID = 0;
};
