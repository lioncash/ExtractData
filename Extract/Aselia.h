#pragma once

#include "ExtractBase.h"

class CAselia final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
};
