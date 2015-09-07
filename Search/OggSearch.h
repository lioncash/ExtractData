#pragma once

class COggSearch final : public CSearchBase
{
public:
	COggSearch();
	void OnInit(SOption* pOption) override;
	void Mount(CArcFile* pclArc) override;
};
