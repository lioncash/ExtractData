#pragma once

class CPngSearch final : public CSearchBase
{
public:
	CPngSearch();
	void Mount(CArcFile* pclArc) override;
};
