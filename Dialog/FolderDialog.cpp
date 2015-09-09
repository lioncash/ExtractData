#include "stdafx.h"
#include "../Common.h"
#include "FolderDialog.h"

BOOL CFolderDialog::DoModal(HWND hWnd, LPCTSTR lpszTitle, LPTSTR pDir)
{
	BROWSEINFO bi = {};
	bi.hwndOwner = hWnd;
	bi.lpfn = reinterpret_cast<BFFCALLBACK>(BrowseCallBackProc);
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	bi.lParam = reinterpret_cast<LPARAM>(pDir);
	bi.lpszTitle = lpszTitle;

	LPITEMIDLIST pItemID = SHBrowseForFolder(&bi);
	if (pItemID == nullptr)
		return FALSE;

	LPMALLOC pMalloc = nullptr;
	if (SHGetMalloc(&pMalloc) == E_FAIL)
	{
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
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
			SendMessage(hWnd, BFFM_SETSELECTION, static_cast<WPARAM>(TRUE), lpData);
			break;
	}

	return 0;
}
