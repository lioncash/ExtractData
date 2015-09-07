#pragma once

#include "HaruotoFD.h"

class Cef_ffd final : public CHaruotoFD
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	void InitMountKey(CArcFile* pclArc) override;
	void InitDecodeKey(CArcFile* pclArc) override;
};
