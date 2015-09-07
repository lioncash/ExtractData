#pragma once

class CWmvSearch final : public CSearchBase
{
public:
	CWmvSearch();
	void Mount(CArcFile* pclArc) override;
};
