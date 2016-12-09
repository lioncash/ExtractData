#include "StdAfx.h"
#include "UI/Dialog/FolderDialog.h"

#include "Common.h"

BOOL CFolderDialog::DoModal(HWND window, LPCTSTR title, LPTSTR directory)
{
	BROWSEINFO bi = {};
	bi.hwndOwner = window;
	bi.lpfn = reinterpret_cast<BFFCALLBACK>(BrowseCallBackProc);
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	bi.lParam = reinterpret_cast<LPARAM>(directory);
	bi.lpszTitle = title;

	LPITEMIDLIST item_id = SHBrowseForFolder(&bi);
	if (item_id == nullptr)
		return FALSE;

	LPMALLOC lp_malloc = nullptr;
	if (SHGetMalloc(&lp_malloc) == E_FAIL)
	{
		CError error;
		error.Message(window, _T("SHGetMalloc Error"));
		return FALSE;
	}

	SHGetPathFromIDList(item_id, directory);

	lp_malloc->Free(item_id);
	lp_malloc->Release();

	return TRUE;
}

LRESULT CALLBACK CFolderDialog::BrowseCallBackProc(HWND window, UINT msg, LPARAM param, LPARAM data)
{
	switch (msg)
	{
		case BFFM_INITIALIZED:
			SendMessage(window, BFFM_SETSELECTION, static_cast<WPARAM>(TRUE), data);
			break;
	}

	return 0;
}
