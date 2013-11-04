#pragma once

#include "../WindowBase.h"
#include "../Common.h"
#include "../Ctrl/Button.h"
#include "../Ctrl/StatusBar.h"
#include "../ArcFile.h"
#include "DBListView.h"

class CDataBase : public CWindowBase
{
private:
	struct UndoRedo
	{
		std::vector<FILEINFODB> entDB;
		std::vector<int> selects;
		int focus;
	};

	TCHAR m_DBFilePath[MAX_PATH];
	HWND m_hWnd;
	std::vector<UndoRedo> m_UndoRedoList;
	bool m_bRedo;

	OPTION* m_pOption;
	CDBListView m_listview;
	CButton m_DelButton;
	CButton m_UndoButton;
	CButton m_AllSelButton;
	CButton m_EndButton;
	CStatusBar m_statusbar;

public:
	CDataBase();
	~CDataBase();

	int ReadFileInfo();
	int DelFileInfo();
	int SaveFileInfo();

	int GetSelItem(std::vector<int>* SelectedItemList);
	int GetUndo();
	int GetRedo();
	int SetUndo();
	void ClearUndoRedo();

	void DoModal(HWND hWnd);
	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	BOOL Check(CArcFile* pArc);
	int Add(CArcFile* pArc);

	void SetOpt(OPTION& option) { m_pOption = &option; }
};
