#pragma once

#include "ExtractBase.h"

class CYkc final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool DecodeYKS(CArcFile* archive);
	bool DecodeYKG(CArcFile* archive);
};
