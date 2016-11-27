#pragma once

#include "Extract/paz/Paz.h"

class CHaruoto : public CPaz
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

protected:
	void InitMountKey(CArcFile* archive) override;
	void InitDecodeKey(CArcFile* archive) override;
};
