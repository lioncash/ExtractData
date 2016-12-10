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
	const std::vector<YCString>& GetHistory() const { return m_open_history_list; }

	void LoadIni() override;
	void SaveIni() override;

	void SetWindowPos(int x, int y, int cx, int cy);

private:
	std::vector<YCString> m_open_history_list;
};

// Search file toolbar
class CSearchToolBar : public CToolBar
{
public:
	HWND Create(HWND hWnd);
	void SetCheckSearch();
	void SetSearch(int nID);
	void SetWindowPos(int x, int y, int cx, int cy);

private:
	int m_begin_id = 0;
};
