#pragma once

#include "ExtractBase.h"

class CWindmill final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;
};
