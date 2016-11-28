#pragma once

#include "UI/Ctrl/Font.h"

class CCtrlBase
{
public:
	CCtrlBase();
	virtual ~CCtrlBase() = default;

	void Init(HWND hWnd, UINT ID);

	virtual HWND Create(HWND hWnd, LPCTSTR lpCaption, UINT ID, int x = 0, int y = 0, int cx = 0, int cy = 0) = 0;

	HWND      GetHandle() const;
	HINSTANCE GetInst() const;
	HWND      GetCtrlHandle() const;
	UINT      GetID() const;
	HFONT     GetFont() const;

	void      SetFont();
	void      SetFocus();
	void      SetWindowPos(int x, int y, int cx, int cy);

	void Enable(BOOL flag);

protected:
	HWND CreateCtrl(LPCTSTR lpClassName, DWORD dwStyle, LPCTSTR lpCaption, int x, int y, int cx, int cy, DWORD dwExStyle = 0);

private:
	HWND      m_hWnd = nullptr;
	HINSTANCE m_hInst = nullptr;
	HWND      m_hCtrl = nullptr;
	UINT      m_ID = 0;
	CFont     m_font;
};
