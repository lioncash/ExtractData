#include "StdAfx.h"
#include "FileDialog.h"

BOOL CFileDialog::DoModal(HWND hWnd, LPTSTR pFileNames, LPTSTR pDir)
{
	OPENFILENAME ofn = {};
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = _T("All files (*.*)\0*.*\0\0");
	ofn.lpstrFile = pFileNames;
	ofn.nFilterIndex = 1;
	ofn.nMaxFile = MAX_PATH * 1000;
	ofn.lpstrInitialDir = pDir;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	ofn.lpstrTitle = _T("Open File");

	if (GetOpenFileName(&ofn) == 0)
		return FALSE;

	GetCurrentDirectory(_MAX_DIR, pDir);

	return TRUE;
}
