#pragma once

class COggSearch : public CSearchBase
{
public:
	COggSearch();
	void OnInit(SOption* pOption);
	void Mount(CArcFile* pclArc);
};
