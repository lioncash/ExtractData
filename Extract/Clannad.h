#pragma once

#include "../ExtractBase.h"

class CClannad final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;
};
