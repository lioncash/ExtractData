#pragma once

class CWindowBase
{
public:
	CWindowBase();
	virtual ~CWindowBase();

	void Init();
	void Init(LONG cx, LONG cy);
	void Init(UINT uID, LONG cx, LONG cy);
	static void Init(HWND hWnd);
	static void Init(HWND hWnd, LONG cx, LONG cy);
	void SaveIni();

	bool Attach(HWND hWnd); // Set
	bool Detach(); // Release

	// Static procedures
	static LRESULT CALLBACK WndStaticProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	// Procedure for overriding
	virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	void MoveWindowCenter();
	void MoveWindowCenter(LONG cx, LONG cy);
	static void MoveWindowCenter(HWND hWnd);
	static void MoveWindowCenter(HWND hWnd, LONG cx, LONG cy);

	POINT GetCenterPt(RECT& dlgrc);
	static POINT GetCenterPt(HWND hWnd, RECT& dlgrc);

protected:
	HWND m_hWnd = nullptr;
	HINSTANCE m_hInst = nullptr;
	WNDPROC m_oldWndProc = nullptr; // Window procedure address
	bool m_bDialog = false; // If dialog, then TRUE

	UINT m_uID = 0;
};
