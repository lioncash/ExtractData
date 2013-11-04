#pragma once

#include <windows.h>
#include <tchar.h>

class CError
{
public:
	void LastError(HWND hWnd);
	void Message(HWND hWnd, LPCTSTR mes, ...);
	void bad_alloc(HWND hWnd);
};
