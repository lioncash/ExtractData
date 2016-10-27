#pragma once

#include "Paz.h"

//----------------------------------------------------------------------------------------
//-- BITTERSWEET FOOLS Repackage ---------------------------------------------------------
//----------------------------------------------------------------------------------------

class CBSFre final : public CPaz
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	void InitMountKey(CArcFile* pclArc) override;
	void InitDecodeKey(CArcFile* pclArc) override;
};
