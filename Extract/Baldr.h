#pragma once

class CBaldr final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
};
