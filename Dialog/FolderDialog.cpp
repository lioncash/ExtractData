#include "stdafx.h"
#include "../Common.h"
#include "FolderDialog.h"

BOOL CFolderDialog::DoModal(HWND hWnd, LPCTSTR lpszTitle, LPTSTR pDir)
{
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = hWnd;
	bi.lpfn = (BFFCALLBACK)BrowseCallBackProc;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	bi.lParam = (LPARAM)pDir;
	bi.lpszTitle = lpszTitle;

	LPITEMIDLIST pItemID = SHBrowseForFolder(&bi);
	if (pItemID == NULL)
		return FALSE;

	LPMALLOC pMalloc = NULL;
	if (SHGetMalloc(&pMalloc) == E_FAIL) {
		CError error;
		error.Message(hWnd, _T("SHGetMalloc Error"));
		return FALSE;
	}

	SHGetPathFromIDList(pItemID, pDir);

	pMalloc->Free(pItemID);
	pMalloc->Release();

	return TRUE;
}

LRESULT CALLBACK CFolderDialog::BrowseCallBackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg) {
		case BFFM_INITIALIZED:
			SendMessage(hWnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
			break;
	}
	return 0;
}
