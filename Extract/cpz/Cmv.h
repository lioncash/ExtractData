#pragma once

#include "../../ExtractBase.h"

class CCmv final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;
};
