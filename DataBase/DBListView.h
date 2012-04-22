#pragma once

#include "../Common.h"
#include "../Ctrl/ListView.h"

class CDBListView : public CListView {
private:
	std::vector<FILEINFODB> m_entDB;

public:
	void Create(HWND hWnd, OPTION& option);
	void Show(NMLVDISPINFO* pDispInfo);

	void Sort();
	void Sort(int column);
	static BOOL CompareFunc(const FILEINFODB& a, const FILEINFODB& b);
	void Update(int StartItem);
};
