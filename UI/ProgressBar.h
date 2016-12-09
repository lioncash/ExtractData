#pragma once

#include "Common.h"

class CProgBar
{
public:
	CProgBar();
	~CProgBar();

	void Init(HWND dlg, QWORD all_file_size);

	void ReplaceFileSize(QWORD old_file_size, QWORD new_file_size);
	void ReplaceAllFileSize(QWORD new_file_size);
	void UpdatePercent(QWORD buffer_size);
	void UpdatePercent();
	void SetArcName(const YCString& archive_name);
	void SetFileName(const YCString& file_name);
	BOOL OnCancel();

	HWND GetHandle()    const { return m_dlg;  }
	HINSTANCE GetInst() const { return m_inst; }

protected:
	HWND m_dlg = nullptr;
	HINSTANCE m_inst = nullptr;
	HWND m_dlg_item_percent = nullptr;
	HWND m_dlg_item_bar = nullptr;
	HWND m_dlg_item_archive = nullptr;
	int m_percent = 0;
	QWORD m_prog_size = 0;
	QWORD m_all_file_size = 0;
};
