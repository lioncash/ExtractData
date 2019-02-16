#pragma once

#include "ExtractBase.h"

class CVA final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountNwa(CArcFile* archive);
	bool MountNwk(CArcFile* archive);
	bool MountOvk(CArcFile* archive);
	bool DecodeNwa(CArcFile* archive);

	int GetBits(u8*& data, int& shift, int bits);
};
