#pragma once

#include "Paz.h"

class CHaruotoFD : public CPaz
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

protected:
	void InitMountKey(CArcFile* pclArc) override;
	void InitDecodeKey(CArcFile* pclArc) override;

	void DecodeTable2() override;

	void DecodeData(void* pvTarget, DWORD dwSize) override;
};
