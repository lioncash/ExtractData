#pragma once

#include "Common.h"
#include "UI/Ctrl/ListView.h"

class CDBListView final : public CListView
{
public:
	void Create(HWND hWnd, SOption& option);
	void Show(NMLVDISPINFO* pDispInfo);

	void Sort();
	void Sort(int column);
	static BOOL CompareFunc(const FILEINFODB& a, const FILEINFODB& b);
	void Update(int StartItem);

private:
	std::vector<FILEINFODB> m_entDB;
};
