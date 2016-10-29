#pragma once

#include "SearchBase.h"

class CMpgSearch final : public CSearchBase
{
public:
	CMpgSearch();
	void Mount(CArcFile* archive) override;
};
