#pragma once

#include "HaruotoFD.h"

class Cef_trial final : public CHaruotoFD
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

protected:
	void InitMountKey(CArcFile* pclArc) override;
	void InitDecodeKey(CArcFile* pclArc) override;
};
