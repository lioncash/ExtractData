#pragma once

#include "Paz.h"

//----------------------------------------------------------------------------------------
//-- BITTERSWEET FOOLS Repackage ---------------------------------------------------------
//----------------------------------------------------------------------------------------

class CBSFre final : public CPaz
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	void InitMountKey(CArcFile* pclArc) override;
	void InitDecodeKey(CArcFile* pclArc) override;
};
