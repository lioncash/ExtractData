#pragma once

#include "Base/YCString.h"
#include "Extract.h"
#include "UI/WindowBase.h"

#include <memory>

class CArcFile;
class CMainListView;
struct SOption;

enum class ExtractMode
{
	Select,
	All
};

class CExtractData : public CWindowBase, public CExtract
{
	using ArchiveVector = std::vector<std::unique_ptr<CArcFile>>;
public:
	CExtractData();
	~CExtractData();

	void Init(HWND hWnd, SOption& option, CMainListView& listview);

	void Open(LPTSTR pOpenDir);
	void OpenDrop(WPARAM wp);
	void OpenHistory(const YCString& file_path);
	void Mount(LPCTSTR c_pclArcNames);

	void SaveSel(LPTSTR pSaveDir, bool convert);
	void SaveAll(LPTSTR pSaveDir, bool convert);
	void SaveDrop();
	void Decode(ExtractMode extract_mode, LPCTSTR pSaveDir, bool convert);
	void DecodeTmp();

	void OpenRelate();
	void DeleteTmpFile();

	void LoadTmpFileList();
	void SaveTmpFileList();

	void Close();

	ArchiveVector& GetArcList() { return m_ArcList; }

	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) override;

private:
	HWND               m_hParentWnd = nullptr;
	HINSTANCE          m_hParentInst = nullptr;
	CMainListView*     m_pListView = nullptr;
	TCHAR              m_szPathToTmpFileList[MAX_PATH];

	LPCTSTR            m_pclArcNames = nullptr;
	ExtractMode        m_dwExtractMode = ExtractMode::Select;
	LPCTSTR            m_pSaveDir = nullptr;
	bool               m_bConvert = false;
	SOption*           m_pOption = nullptr;
  bool               m_bInput = false;
	ArchiveVector      m_ArcList;

	std::set<YCString> m_ssTmpFile;

	void Save(ExtractMode extract_mode, LPTSTR pSaveDir, bool convert);
	static UINT WINAPI MountThread(LPVOID lpParam);
	static UINT WINAPI DecodeThread(LPVOID lpParam);
};
