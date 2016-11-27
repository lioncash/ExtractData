#pragma once

#include "Extract/paz/HaruotoFD.h"

class Cef_trial final : public CHaruotoFD
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

protected:
	void InitMountKey(CArcFile* archive) override;
	void InitDecodeKey(CArcFile* archive) override;
};
