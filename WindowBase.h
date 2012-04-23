#pragma once

class CWindowBase {

protected:
    HWND m_hWnd;
    HINSTANCE m_hInst;
    WNDPROC m_oldWndProc; // Window procedure address
    BOOL m_bDialog; // If dialog, then TRUE

    UINT m_uID;

public:
    CWindowBase();
    virtual ~CWindowBase();

    void Init();
    void Init(LONG cx, LONG cy);
    void Init(UINT uID, LONG cx, LONG cy);
    static void Init(HWND hWnd);
    static void Init(HWND hWnd, LONG cx, LONG cy);
    void SaveIni();

    BOOL Attach(HWND hWnd); // Set
    BOOL Detach(); // Release

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
};
