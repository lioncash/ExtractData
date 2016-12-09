#include "StdAfx.h"
#include "UI/Dialog/ExistsDialog.h"

#include "res/ResExtractData.h"

DWORD CExistsDialog::m_overwrite;

void CExistsDialog::DoModal(HWND window, LPCTSTR file_path)
{
	if (m_overwrite == 0x01)
	{
		m_file_path = file_path;
		HINSTANCE inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(window, GWLP_HINSTANCE));
		INT_PTR ret = DialogBoxParam(inst, _T("EXISTSDLG"), window, reinterpret_cast<DLGPROC>(WndStaticProc), reinterpret_cast<LPARAM>(this));
		if (ret == IDCANCEL)
			throw CExistsDialog();
	}
	else if (m_overwrite == 0x10)
	{
		throw CExistsDialog();
	}
}

LRESULT CExistsDialog::WndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			Init();
			SetWindowText(GetDlgItem(window, IDC_EXISTS_PATH), m_file_path);
			SetFocus(GetDlgItem(window, IDC_EXISTS_YES));

			return FALSE;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wp))
			{
				case IDC_EXISTS_YES:
					EndDialog(window, IDOK);
					return TRUE;

				case IDC_EXISTS_NO:
				case IDCANCEL:
					EndDialog(window, IDCANCEL);
					return TRUE;

				case IDC_EXISTS_ALLYES:
					m_overwrite = 0x00;
					EndDialog(window, IDOK);
					return TRUE;

				case IDC_EXISTS_ALLNO:
					m_overwrite = 0x10;
					EndDialog(window, IDCANCEL);
					return TRUE;
			}
			return FALSE;
		}
	}

	return FALSE;
}
