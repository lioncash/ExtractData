#pragma once

#include "SearchBase.h"

class CBmpSearch final : public CSearchBase
{
public:
	CBmpSearch();
	void Mount(CArcFile* archive) override;
};
