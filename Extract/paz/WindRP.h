#pragma once

#include "Extract/paz/Haruoto.h"

class CWindRP final : public CHaruoto
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

protected:
	void InitMountKey(CArcFile* archive) override;
	void InitDecodeKey(CArcFile* archive) override;
};
