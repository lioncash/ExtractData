#pragma once

#include "Extract/paz/Paz.h"

class CHaruotoFD : public CPaz
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

protected:
	void InitMountKey(CArcFile* archive) override;
	void InitDecodeKey(CArcFile* archive) override;

	void DecodeTable2() override;

	void DecodeData(u8* target, size_t size) override;
};
