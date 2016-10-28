#pragma once

#include "SearchBase.h"

//----------------------------------------------------------------------------------------
//-- MIDI Search Class -------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class CMidSearch final : public CSearchBase
{
public:
	CMidSearch();
	void Mount(CArcFile* pclArc) override;
};
