#include "stdafx.h"
#include "Error.h"

extern BOOL g_bThreadEnd;

// Function that displays the error message obtained by GetLastError()
void CError::LastError(HWND hWnd)
{
	g_bThreadEnd = TRUE;
	LPTSTR lpMsgBuf = NULL;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		lpMsgBuf,
		0,
		NULL);

	if (lpMsgBuf)
	{
		MessageBox(hWnd, lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION);
		LocalFree(lpMsgBuf);
	}
}

void CError::Message(HWND hWnd, LPCTSTR mes, ...)
{
	TCHAR string[1024];
	va_list list;

	va_start(list, mes);
	wvsprintf(string, mes, list);
	va_end(list);

	g_bThreadEnd = TRUE;
	MessageBox(hWnd, string, _T("Error"), MB_OK | MB_ICONSTOP);
}

void CError::bad_alloc(HWND hWnd)
{
	g_bThreadEnd = TRUE;
	Message(hWnd, _T("You do not have enough free memory..."));
}