#pragma once

class CFolderDialog
{
public:
	BOOL DoModal(HWND window, LPCTSTR title, LPTSTR directory);

private:
	static LRESULT CALLBACK BrowseCallBackProc(HWND window, UINT msg, LPARAM param, LPARAM data);
};
