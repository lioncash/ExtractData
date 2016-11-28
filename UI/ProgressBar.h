#pragma once

#include "Common.h"

class CProgBar
{
public:
	CProgBar();
	~CProgBar();

	void Init(HWND hDlg, QWORD AllFileSize);

	void ReplaceFileSize(QWORD oldFileSize, QWORD newFileSize);
	void ReplaceAllFileSize(QWORD newFileSize);
	void UpdatePercent(QWORD BufSize);
	void UpdatePercent();
	void SetArcName(const YCString& pclArcName);
	void SetFileName(const YCString& pFileName);
	BOOL OnCancel();

	HWND GetHandle()    const { return m_hDlg;  }
	HINSTANCE GetInst() const { return m_hInst; }

protected:
	HWND m_hDlg = nullptr;
	HINSTANCE m_hInst = nullptr;
	HWND m_hDlgItem_percent = nullptr;
	HWND m_hDlgItem_bar = nullptr;
	HWND m_hDlgItem_archive = nullptr;
	int m_percent = 0;
	QWORD m_ProgSize = 0;
	QWORD m_AllFileSize = 0;
};
