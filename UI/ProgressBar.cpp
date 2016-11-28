#include "StdAfx.h"
#include "UI/ProgressBar.h"

#include "res/ResExtractData.h"

extern BOOL g_bThreadEnd;

CProgBar::CProgBar()
{
}

CProgBar::~CProgBar()
{
}

void CProgBar::Init(HWND hDlg, QWORD AllFileSize)
{
	m_hDlg = hDlg;
	m_hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hDlg, GWLP_HINSTANCE));
	m_AllFileSize = AllFileSize;
	m_hDlgItem_percent = GetDlgItem(hDlg, IDC_PERCENT);
	m_hDlgItem_bar = GetDlgItem(hDlg, IDC_PROGBAR1);
	m_hDlgItem_archive = GetDlgItem(hDlg, IDC_EXTFILENAME);
	SendMessage(m_hDlgItem_bar, PBM_SETRANGE, static_cast<WPARAM>(0), MAKELPARAM(0, 100));
	// Show 0%
	UpdatePercent(0);
}

void CProgBar::ReplaceFileSize(QWORD oldFileSize, QWORD newFileSize)
{
	m_AllFileSize = m_AllFileSize - oldFileSize + newFileSize;
}

void CProgBar::ReplaceAllFileSize(QWORD newFileSize)
{
	m_AllFileSize += newFileSize;
}

// Function to update the percentage
void CProgBar::UpdatePercent(QWORD BufSize)
{
	LPQWORD pProgSize = &m_ProgSize;
	*pProgSize += BufSize;
	int percent = static_cast<double>(*pProgSize) / m_AllFileSize * 100;
	
	if (percent > m_percent)
	{
		m_percent = percent;
		TCHAR percent_str[256];
		_stprintf(percent_str, _T("%3d%%"), percent);
		SetWindowText(m_hDlgItem_percent, percent_str);
		SendMessage(m_hDlgItem_bar, PBM_SETPOS, percent, 0);
	}

	if (OnCancel())
		throw -1;
}

// Function that updates to 100%
void CProgBar::UpdatePercent()
{
	UpdatePercent(m_AllFileSize - m_ProgSize);
}

void CProgBar::SetArcName(const YCString& pclArcName)
{
	// Show the archive file name
	SetWindowText(m_hDlgItem_archive, pclArcName);
}

void CProgBar::SetFileName(const YCString& pFileName)
{
	// Show the file name
	SetWindowText(m_hDlgItem_archive, pFileName);
}

// Function that is executed on canceling
BOOL CProgBar::OnCancel()
{
	if (g_bThreadEnd)
		return TRUE;

	return FALSE;
}
