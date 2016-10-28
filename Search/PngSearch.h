#pragma once

#include "SearchBase.h"

class CPngSearch final : public CSearchBase
{
public:
	CPngSearch();
	void Mount(CArcFile* pclArc) override;
};
