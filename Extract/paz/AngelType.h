#pragma once

#include "Paz.h"

class CAngelType final : public CPaz
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

protected:
	void InitMountKey(CArcFile* pclArc) override;
	void InitDecodeKey(CArcFile* pclArc) override;
};
