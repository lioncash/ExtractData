#pragma once

#include "../ExtractBase.h"

class CYkc final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	bool DecodeYKS(CArcFile* pclArc);
	bool DecodeYKG(CArcFile* pclArc);
};
