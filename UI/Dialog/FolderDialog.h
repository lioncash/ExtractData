#pragma once

class CFolderDialog
{
public:
	BOOL DoModal(HWND window, LPCTSTR title, LPTSTR directory);

private:
	static int CALLBACK BrowseCallBackProc(HWND window, UINT msg, LPARAM param, LPARAM data);
};
