#pragma once

#include "../ExtractBase.h"

class CIzumo final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
};
