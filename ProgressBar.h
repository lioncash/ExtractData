#pragma once

#include "Common.h"

class CProgBar
{
protected:
	HWND m_hDlg;
	HINSTANCE m_hInst;
	HWND m_hDlgItem_percent;
	HWND m_hDlgItem_bar;
	HWND m_hDlgItem_archive;
	int m_percent;
	QWORD m_ProgSize;
	QWORD m_AllFileSize;

public:
	CProgBar();
	~CProgBar();
	
	void Init(HWND hDlg, QWORD AllFileSize);

	void ReplaceFileSize(QWORD oldFileSize, QWORD newFileSize);
	void ReplaceAllFileSize(QWORD newFileSize);
	void UpdatePercent(QWORD BufSize);
	void UpdatePercent();
	void SetArcName(YCString& pclArcName);
	void SetFileName(YCString& pFileName);
	BOOL OnCancel();

	HWND GetHandle() { return m_hDlg; }
	HINSTANCE GetInst() { return m_hInst; }
};
