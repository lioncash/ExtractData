#pragma once

#include "Common.h"
#include "res/ResExtractData.h"
#include "MainListView.h"
#include "ProgressBar.h"
#include "Extract.h"
#include "WindowBase.h"

#define EXTRACT_SELECT	0
#define EXTRACT_ALL		1

class CExtractData : public CWindowBase, public CExtract {
private:
	HWND m_hParentWnd;
	HINSTANCE m_hParentInst;
	CMainListView* m_pListView;
	TCHAR m_szPathToTmpFileList[MAX_PATH];

	LPCTSTR m_pclArcNames;
	DWORD m_dwExtractMode;
	LPCTSTR m_pSaveDir;
	BOOL	m_bConvert;
	SOption* m_pOption;
	BOOL m_bInput;
	std::vector<CArcFile*> m_ArcList;

	std::set<YCString> m_ssTmpFile;

	void Save(DWORD ExtractMode, LPTSTR pSaveDir, BOOL bConvert);
	static UINT WINAPI MountThread(LPVOID lpParam);
	static UINT WINAPI DecodeThread(LPVOID lpParam);

public:
	CExtractData();
	void Init(HWND hWnd, SOption& option, CMainListView& listview);

	void Open(LPTSTR pOpenDir);
	void OpenDrop(WPARAM wp);
	void OpenHistory(YCString& sFilePath);
	void Mount(LPCTSTR c_pclArcNames);

	void SaveSel(LPTSTR pSaveDir, BOOL bConvert);
	void SaveAll(LPTSTR pSaveDir, BOOL bConvert);
	void SaveDrop();
	void Decode(DWORD ExtractMode, LPCTSTR pSaveDir, BOOL bConvert);
	void DecodeTmp();

	void OpenRelate();
	void DeleteTmpFile();

	void LoadTmpFileList();
	void SaveTmpFileList();

	void Close();

	std::vector<CArcFile*>& GetArcList() { return (m_ArcList); }

	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};