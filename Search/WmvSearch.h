#pragma once

#include "SearchBase.h"

class CWmvSearch final : public CSearchBase
{
public:
	CWmvSearch();
	void Mount(CArcFile* pclArc) override;
};
