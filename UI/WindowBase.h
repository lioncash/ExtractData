#pragma once

class CWindowBase
{
public:
	CWindowBase();
	virtual ~CWindowBase();

	void Init();
	void Init(LONG cx, LONG cy);
	void Init(UINT uID, LONG cx, LONG cy);
	static void Init(HWND window);
	static void Init(HWND window, LONG cx, LONG cy);
	void SaveIni();

	bool Attach(HWND window); // Set
	bool Detach(); // Release

	// Static procedures
	static LRESULT CALLBACK WndStaticProc(HWND window, UINT msg, WPARAM wp, LPARAM lp);
	// Procedure for overriding
	virtual LRESULT WndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp);

	void MoveWindowCenter();
	void MoveWindowCenter(LONG cx, LONG cy);
	static void MoveWindowCenter(HWND window);
	static void MoveWindowCenter(HWND window, LONG cx, LONG cy);

	POINT GetCenterPt(const RECT& dlgrc);
	static POINT GetCenterPt(HWND window, const RECT& dlgrc);

protected:
	HWND m_window = nullptr;
	HINSTANCE m_inst = nullptr;
	WNDPROC m_old_window_proc = nullptr; // Window procedure address
	bool m_is_dialog = false; // If dialog, then TRUE

	UINT m_id = 0;
};
