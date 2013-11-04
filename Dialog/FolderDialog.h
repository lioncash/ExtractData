#pragma once

class CFolderDialog
{
private:
	static LRESULT CALLBACK BrowseCallBackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

public:
	BOOL DoModal(HWND hWnd, LPCTSTR lpszTitle, LPTSTR pDir);
};
