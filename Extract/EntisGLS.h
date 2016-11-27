#pragma once

#include "ExtractBase.h"

class CEntisGLS final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;
};
