#include "stdafx.h"
#include "../res/ResExtractData.h"
#include "ExistsDialog.h"

DWORD CExistsDialog::m_fOverWrite;

void CExistsDialog::DoModal(HWND hWnd, LPCTSTR pFilePath)
{
	if (m_fOverWrite == 0x01) {
		m_pFilePath = pFilePath;
		HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
		INT_PTR ret = DialogBoxParam(hInst, _T("EXISTSDLG"), hWnd, (DLGPROC)WndStaticProc, (LPARAM)this);
		if (ret == IDCANCEL)
			throw CExistsDialog();
	}
	else if (m_fOverWrite == 0x10) {
		throw CExistsDialog();
	}
}

LRESULT CExistsDialog::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
		case WM_INITDIALOG:
		{
			Init();
			SetWindowText(GetDlgItem(hWnd, IDC_EXISTS_PATH), m_pFilePath);
			SetFocus(GetDlgItem(hWnd, IDC_EXISTS_YES));

			return FALSE;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wp)) {
				case IDC_EXISTS_YES:
					EndDialog(hWnd, IDOK);
					return TRUE;

				case IDC_EXISTS_NO:
				case IDCANCEL:
					EndDialog(hWnd, IDCANCEL);
					return TRUE;

				case IDC_EXISTS_ALLYES:
					m_fOverWrite = 0x00;
					EndDialog(hWnd, IDOK);
					return TRUE;

				case IDC_EXISTS_ALLNO:
					m_fOverWrite = 0x10;
					EndDialog(hWnd, IDCANCEL);
					return TRUE;
			}
			return FALSE;
		}
	}

	return FALSE;
}
