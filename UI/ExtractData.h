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

	void Init(HWND parent, SOption& option, CMainListView& listview);

	void Open(LPTSTR open_dir);
	void OpenDrop(WPARAM wp);
	void OpenHistory(const YCString& file_path);
	void Mount(LPCTSTR archive_names);

	void SaveSel(LPTSTR pSaveDir, bool convert);
	void SaveAll(LPTSTR pSaveDir, bool convert);
	void SaveDrop();
	void Decode(ExtractMode extract_mode, LPCTSTR save_dir, bool convert);
	void DecodeTmp();

	void OpenRelate();
	void DeleteTmpFile();

	void LoadTmpFileList();
	void SaveTmpFileList();

	void Close();

	ArchiveVector& GetArcList() { return m_archives; }

	LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) override;

private:
	HWND               m_parent_window = nullptr;
	HINSTANCE          m_parent_inst = nullptr;
	CMainListView*     m_list_view = nullptr;
	TCHAR              m_tmp_file_list_path[MAX_PATH];

	LPCTSTR            m_archive_names = nullptr;
	ExtractMode        m_extract_mode = ExtractMode::Select;
	LPCTSTR            m_save_dir = nullptr;
	bool               m_convert = false;
	SOption*           m_options = nullptr;
  bool               m_input = false;
	ArchiveVector      m_archives;

	std::set<YCString> m_tmp_file_paths;

	void Save(ExtractMode extract_mode, LPTSTR save_dir, bool convert);
	static UINT WINAPI MountThread(LPVOID param);
	static UINT WINAPI DecodeThread(LPVOID param);
};
