#pragma once

#include "SearchBase.h"

class CJpgSearch final : public CSearchBase
{
public:
	CJpgSearch();
	void Mount(CArcFile* pclArc) override;
};
