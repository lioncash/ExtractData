#pragma once

#include "ExtractBase.h"

class CBaldr final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
};
