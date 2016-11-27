#pragma once

#include "Extract/paz/Paz.h"

//----------------------------------------------------------------------------------------
//-- BITTERSWEET FOOLS Repackage ---------------------------------------------------------
//----------------------------------------------------------------------------------------

class CBSFre final : public CPaz
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	void InitMountKey(CArcFile* archive) override;
	void InitDecodeKey(CArcFile* archive) override;
};
