#pragma once

#include "../../ExtractBase.h"

class CCmv final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;
};
