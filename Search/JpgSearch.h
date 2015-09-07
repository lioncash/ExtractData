#pragma once

class CJpgSearch final : public CSearchBase
{
public:
	CJpgSearch();
	void Mount(CArcFile* pclArc) override;
};
