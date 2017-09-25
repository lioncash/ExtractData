#include "StdAfx.h"
#include "UI/Dialog/FolderInputDialog.h"

#include "UI/Ctrl/Button.h"
#include "UI/Ctrl/EditBox.h"
#include "UI/Ctrl/Label.h"
#include "UI/Dialog/FolderDialog.h"
#include "UI/Option.h"

INT_PTR CFolderInputDialog::DoModal(HWND window, LPTSTR save_dir)
{
	m_save_dir = save_dir;
	HINSTANCE inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(window, GWLP_HINSTANCE));
	return DialogBoxParam(inst, _T("FOLDER_INPUT_DLG"), window, reinterpret_cast<DLGPROC>(WndStaticProc), reinterpret_cast<LPARAM>(this));
}

LRESULT CFolderInputDialog::WndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
	static CButton btn_dir, btn_ok, btn_cancel;
	static CEditBox edit_dir;
	static CLabel label_dir;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			// Allow D&D (Drag & Drop)
			DragAcceptFiles(window, TRUE);

			UINT id = 10000;
			int x = 10;
			int y = -10;

			SetWindowText(window, _T("Select a destination"));

			label_dir.Create(window, _T("Input folder name"), id++, x, y += 20, 100, 20);
			edit_dir.Create(window, m_save_dir, id++, x, y += 20, 300, 22);
			btn_dir.Create(window, _T("Browse"), id++, x + 300, y + 1, 40, 20);

			btn_ok.Create(window, _T("OK"), IDOK, 110, y += 30, 70, 23);
			btn_cancel.Create(window, _T("Cancel"), IDCANCEL, 190, y, 70, 23);

			edit_dir.SetFocus();
			btn_ok.SetDef();

			Init(380, y + 60);

			return FALSE;
		}

		case WM_DROPFILES:
		{
			TCHAR save_dir[_MAX_DIR];
			HDROP drop = reinterpret_cast<HDROP>(wp);
			DragQueryFile(drop, 0, save_dir, sizeof(save_dir));
			edit_dir.SetText(save_dir);
			return FALSE;
		}

		case WM_COMMAND:

			//  Browse button is pressed
			if (LOWORD(wp) == btn_dir.GetID())
			{
				TCHAR save_dir[_MAX_DIR];
				lstrcpy(save_dir, m_save_dir);
				CFolderDialog folder_dialog;
				if (folder_dialog.DoModal(window, _T("Select a folder"), save_dir))
					edit_dir.SetText(save_dir);
				return FALSE;
			}

			// OK button is pressed
			if (LOWORD(wp) == IDOK)
			{
				edit_dir.GetText(m_save_dir, _MAX_DIR);
				PathRemoveBackslash(m_save_dir);
				EndDialog(window, IDOK);
				return TRUE;
			}

			// Cancel button is pressed
			if (LOWORD(wp) == IDCANCEL)
			{
				EndDialog(window, IDCANCEL);
				return TRUE;
			}

			return FALSE;
	}

	return FALSE;
}
