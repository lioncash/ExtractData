#pragma once

#include "SearchBase.h"

class COggSearch final : public CSearchBase
{
public:
	COggSearch();
	void OnInit(SOption* option) override;
	void Mount(CArcFile* archive) override;
};
