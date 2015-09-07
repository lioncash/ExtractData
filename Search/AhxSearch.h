#pragma once

class CAhxSearch final : public CSearchBase
{
public:
	CAhxSearch();
	void Mount(CArcFile* pclArc) override;
};
