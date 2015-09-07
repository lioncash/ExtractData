#pragma once

class CMpgSearch final : public CSearchBase
{
public:
	CMpgSearch();
	void Mount(CArcFile* pclArc) override;
};
