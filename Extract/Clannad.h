#pragma once

#include "../ExtractBase.h"

class CClannad final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;
};
