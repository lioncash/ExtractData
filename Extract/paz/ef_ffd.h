#pragma once

#include "HaruotoFD.h"

class Cef_ffd final : public CHaruotoFD
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	void InitMountKey(CArcFile* archive) override;
	void InitDecodeKey(CArcFile* archive) override;
};
