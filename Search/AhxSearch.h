#pragma once

#include "SearchBase.h"

class CAhxSearch final : public CSearchBase
{
public:
	CAhxSearch();
	void Mount(CArcFile* archive) override;
};
