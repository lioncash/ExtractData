#pragma once

class CBmpSearch final : public CSearchBase
{
public:
	CBmpSearch();
	void Mount(CArcFile* pclArc) override;
};
