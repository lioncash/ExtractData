#pragma once

#include "UI/Ctrl/Font.h"

class CCtrlBase
{
public:
	CCtrlBase();
	virtual ~CCtrlBase() = default;

	void Init(HWND window, UINT id);

	virtual HWND Create(HWND window, LPCTSTR caption, UINT id, int x = 0, int y = 0, int cx = 0, int cy = 0) = 0;

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
	HWND CreateCtrl(LPCTSTR class_name, DWORD style, LPCTSTR caption, int x, int y, int cx, int cy, DWORD ex_style = 0);

private:
	HWND      m_window = nullptr;
	HINSTANCE m_inst = nullptr;
	HWND      m_ctrl = nullptr;
	UINT      m_id = 0;
	CFont     m_font;
};
