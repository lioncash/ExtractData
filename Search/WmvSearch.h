#pragma once

class CWmvSearch : public CSearchBase {
public:
	CWmvSearch();
	void Mount(CArcFile* pclArc);
};