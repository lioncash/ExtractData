#include "StdAfx.h"
#include "UI/Dialog/SearchDialog.h"

#include "res/ResExtractData.h"
#include "UI/Option.h"

int CSearchDialog::DoModal(HWND window, LPCTSTR archive_path)
{
	m_archive_path = archive_path;
	HINSTANCE inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(window, GWLP_HINSTANCE));
	return DialogBoxParam(inst, _T("SEARCHDLG"), window, reinterpret_cast<DLGPROC>(WndStaticProc), reinterpret_cast<LPARAM>(this));
}

LRESULT CSearchDialog::WndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
	static COption option;

	switch (msg)
	{
		case WM_INITDIALOG:
			Init();
			TCHAR message[1024];
			_stprintf(message, _T("%s is an unsupported file. \n Do you want to search through it anyway?"), m_archive_path);
			SetWindowText(GetDlgItem(window, IDC_SEARCH_MESSAGE), message);
			return FALSE;

		case WM_COMMAND:
		{
			switch (LOWORD(wp))
			{
				case IDC_SEARCH_YES:
					EndDialog(window, IDYES);
					return TRUE;

				case IDC_SEARCH_NO:
				case IDCANCEL:
					EndDialog(window, IDNO);
					return TRUE;

				case IDC_SEARCH_OPTION:
					option.DoModal(window);
					return FALSE;
			}
			return FALSE;
		}
	}

	return FALSE;
}
