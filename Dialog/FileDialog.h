#pragma once

class CFileDialog
{
public:
	static BOOL DoModal(HWND hWnd, LPTSTR pFileNames, LPTSTR LastReadFileDir);
};
