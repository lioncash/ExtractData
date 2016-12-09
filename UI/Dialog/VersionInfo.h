#pragma once

#include "UI/WindowBase.h"

class CVersionInfo : public CWindowBase
{
public:
	void DoModal(HWND window);
	LRESULT WndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp) override;
};
