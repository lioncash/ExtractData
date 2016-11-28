#pragma once

#include "UI/WindowBase.h"

class CVersionInfo : public CWindowBase
{
public:
	void DoModal(HWND hWnd);
	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) override;
};
