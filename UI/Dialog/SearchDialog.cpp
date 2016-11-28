#include "StdAfx.h"
#include "UI/Dialog/SearchDialog.h"

#include "res/ResExtractData.h"
#include "UI/Option.h"

int CSearchDialog::DoModal(HWND hWnd, LPCTSTR pclArcPath)
{
	m_pclArcPath = pclArcPath;
	HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
	return DialogBoxParam(hInst, _T("SEARCHDLG"), hWnd, reinterpret_cast<DLGPROC>(WndStaticProc), reinterpret_cast<LPARAM>(this));
}

LRESULT CSearchDialog::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static COption option;

	switch (msg)
	{
		case WM_INITDIALOG:
			Init();
			TCHAR message[1024];
			_stprintf(message, _T("%s is an unsupported file. \n Do you want to search through it anyway?"), m_pclArcPath);
			SetWindowText(GetDlgItem(hWnd, IDC_SEARCH_MESSAGE), message);
			return FALSE;

		case WM_COMMAND:
		{
			switch (LOWORD(wp))
			{
				case IDC_SEARCH_YES:
					EndDialog(hWnd, IDYES);
					return TRUE;

				case IDC_SEARCH_NO:
				case IDCANCEL:
					EndDialog(hWnd, IDNO);
					return TRUE;

				case IDC_SEARCH_OPTION:
					option.DoModal(hWnd);
					return FALSE;
			}
			return FALSE;
		}
	}

	return FALSE;
}
