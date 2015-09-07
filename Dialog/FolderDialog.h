#pragma once

class CFolderDialog
{
public:
	BOOL DoModal(HWND hWnd, LPCTSTR lpszTitle, LPTSTR pDir);

private:
	static LRESULT CALLBACK BrowseCallBackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
};
