#include "StdAfx.h"
#include "UI/Dialog/FileDialog.h"

BOOL CFileDialog::DoModal(HWND window, LPTSTR file_names, LPTSTR dir)
{
	OPENFILENAME ofn = {};
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = window;
	ofn.lpstrFilter = _T("All files (*.*)\0*.*\0\0");
	ofn.lpstrFile = file_names;
	ofn.nFilterIndex = 1;
	ofn.nMaxFile = MAX_PATH * 1000;
	ofn.lpstrInitialDir = dir;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	ofn.lpstrTitle = _T("Open File");

	if (GetOpenFileName(&ofn) == 0)
		return FALSE;

	GetCurrentDirectory(_MAX_DIR, dir);

	return TRUE;
}
