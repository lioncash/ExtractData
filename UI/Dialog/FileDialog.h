#pragma once

class CFileDialog
{
public:
	static BOOL DoModal(HWND window, LPTSTR file_names, LPTSTR last_read_file_dir);
};
