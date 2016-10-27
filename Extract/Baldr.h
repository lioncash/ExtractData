#pragma once

class CBaldr final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
};
