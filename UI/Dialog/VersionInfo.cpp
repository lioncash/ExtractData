#include "StdAfx.h"
#include "UI/Dialog/VersionInfo.h"

#include "res/ResExtractData.h"

void CVersionInfo::DoModal(HWND window)
{
	HINSTANCE inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(window, GWLP_HINSTANCE));
	DialogBoxParam(inst, _T("MYVERSION"), window, reinterpret_cast<DLGPROC>(WndStaticProc), reinterpret_cast<LPARAM>(this));
}

// Version info dialog
LRESULT CVersionInfo::WndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			Init();
			SetFocus(GetDlgItem(window, IDC_OKBUTTON));
			return FALSE;

		case WM_COMMAND:
		{
			switch (LOWORD(wp))
			{
				case IDC_OKBUTTON:
					EndDialog(window, IDOK);
					return TRUE;

				case IDCANCEL:
					EndDialog(window, IDCANCEL);
					return TRUE;
			}
			return FALSE;
		}
	}

	return FALSE;
}