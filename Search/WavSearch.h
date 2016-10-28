#pragma once

#include "SearchBase.h"

class CWavSearch final : public CSearchBase
{
public:
	CWavSearch();
	void Mount(CArcFile* pclArc) override;
};
