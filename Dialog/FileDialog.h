#pragma once

class CFileDialog {
public:
	static BOOL DoModal(HWND hWnd, LPTSTR pFileNames, LPTSTR LastReadFileDir);
	static LRESULT CALLBACK BrowseCallBackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
};
