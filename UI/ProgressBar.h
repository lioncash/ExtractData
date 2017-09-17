#pragma once

#include "Common.h"

class CProgBar
{
public:
	CProgBar();
	~CProgBar();

	void Init(HWND dlg, u64 all_file_size);

	void ReplaceFileSize(u64 old_file_size, u64 new_file_size);
	void ReplaceAllFileSize(u64 new_file_size);
	void UpdatePercent(u64 buffer_size);
	void UpdatePercent();
	void SetArcName(const YCString& archive_name);
	void SetFileName(const YCString& file_name);
	bool OnCancel();

	HWND GetHandle()    const { return m_dlg;  }
	HINSTANCE GetInst() const { return m_inst; }

protected:
	HWND m_dlg = nullptr;
	HINSTANCE m_inst = nullptr;
	HWND m_dlg_item_percent = nullptr;
	HWND m_dlg_item_bar = nullptr;
	HWND m_dlg_item_archive = nullptr;
	int m_percent = 0;
	u64 m_prog_size = 0;
	u64 m_all_file_size = 0;
};
