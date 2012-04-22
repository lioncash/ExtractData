#include "stdafx.h"
#include "../Ctrl/Button.h"
#include "../Ctrl/EditBox.h"
#include "../Ctrl/Label.h"
#include "../Option.h"
#include "FolderDialog.h"
#include "FolderInputDialog.h"

INT_PTR CFolderInputDialog::DoModal(HWND hWnd, LPTSTR pSaveDir)
{
	m_pSaveDir = pSaveDir;
	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	INT_PTR ret = DialogBoxParam(hInst, _T("FOLDER_INPUT_DLG"), hWnd, (DLGPROC)WndStaticProc, (LPARAM)this);
	return (ret);
}

LRESULT CFolderInputDialog::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static CButton BtnDir, BtnOk, BtnCancel;
	static CEditBox EditDir;
	static CLabel LabelDir;

	switch (msg) {
		case WM_INITDIALOG:
		{
			// Allow D&D (Drag & Drop)
			DragAcceptFiles(hWnd, TRUE);

			UINT ID = 10000;
			int x = 10, y = -10;

			SetWindowText(hWnd, _T("Select a destination"));

			LabelDir.Create(hWnd, _T("Input folder name"), ID++, x, y += 20, 100, 20);
			EditDir.Create(hWnd, m_pSaveDir, ID++, x, y += 20, 300, 22);
			BtnDir.Create(hWnd, _T("Browse"), ID++, x + 300, y + 1, 40, 20);

			BtnOk.Create(hWnd, _T("OK"), IDOK, 110, y += 30, 70, 23);
			BtnCancel.Create(hWnd, _T("Cancel"), IDCANCEL, 190, y, 70, 23);

			EditDir.SetFocus();
			BtnOk.SetDef();

			Init(380, y + 60);

			return FALSE;
		}

		case WM_DROPFILES:
		{
			TCHAR szSaveDir[_MAX_DIR];
			HDROP hDrop = (HDROP)wp;
			DragQueryFile(hDrop, 0, szSaveDir, sizeof(szSaveDir));
			EditDir.SetText(szSaveDir);
			return FALSE;
		}

		case WM_COMMAND:
			//  Browse button is pressed
			if (LOWORD(wp) == BtnDir.GetID()) {
				TCHAR szSaveDir[_MAX_DIR];
				lstrcpy(szSaveDir, m_pSaveDir);
				CFolderDialog FolderDlg;
				if (FolderDlg.DoModal(hWnd, _T("Select a folder"), szSaveDir) == TRUE)
					EditDir.SetText(szSaveDir);
				return FALSE;
			}
			// OK button is pressed
			if (LOWORD(wp) == IDOK) {
				EditDir.GetText(m_pSaveDir, _MAX_DIR);
				PathRemoveBackslash(m_pSaveDir);
				EndDialog(hWnd, IDOK);
				return TRUE;
			}
			// Cancel button is pressed
			if (LOWORD(wp) == IDCANCEL) {
				EndDialog(hWnd, IDCANCEL);
				return TRUE;
			}
			return FALSE;
	}

	return FALSE;
}
