#include "StdAfx.h"
#include "UI/ProgressBar.h"

#include "res/ResExtractData.h"

extern bool g_bThreadEnd;

CProgBar::CProgBar()
{
}

CProgBar::~CProgBar()
{
}

void CProgBar::Init(HWND dlg, u64 all_file_size)
{
	m_dlg = dlg;
	m_inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(dlg, GWLP_HINSTANCE));
	m_all_file_size = all_file_size;
	m_dlg_item_percent = GetDlgItem(dlg, IDC_PERCENT);
	m_dlg_item_bar = GetDlgItem(dlg, IDC_PROGBAR1);
	m_dlg_item_archive = GetDlgItem(dlg, IDC_EXTFILENAME);
	SendMessage(m_dlg_item_bar, PBM_SETRANGE, static_cast<WPARAM>(0), MAKELPARAM(0, 100));
	// Show 0%
	UpdatePercent(0);
}

void CProgBar::ReplaceFileSize(u64 old_file_size, u64 new_file_size)
{
	m_all_file_size = m_all_file_size - old_file_size + new_file_size;
}

void CProgBar::ReplaceAllFileSize(u64 new_file_size)
{
	m_all_file_size += new_file_size;
}

// Function to update the percentage
void CProgBar::UpdatePercent(u64 buffer_size)
{
	m_prog_size += buffer_size;
	const int percent = static_cast<double>(m_prog_size) / m_all_file_size * 100;

	if (percent > m_percent)
	{
		m_percent = percent;
		TCHAR percent_str[256];
		_stprintf(percent_str, _T("%3d%%"), percent);
		SetWindowText(m_dlg_item_percent, percent_str);
		SendMessage(m_dlg_item_bar, PBM_SETPOS, percent, 0);
	}

	if (OnCancel())
		throw -1;
}

// Function that updates to 100%
void CProgBar::UpdatePercent()
{
	UpdatePercent(m_all_file_size - m_prog_size);
}

void CProgBar::SetArcName(const YCString& archive_name)
{
	// Show the archive file name
	SetWindowText(m_dlg_item_archive, archive_name);
}

void CProgBar::SetFileName(const YCString& file_name)
{
	// Show the file name
	SetWindowText(m_dlg_item_archive, file_name);
}

// Function that is executed on canceling
bool CProgBar::OnCancel()
{
	return g_bThreadEnd;
}
