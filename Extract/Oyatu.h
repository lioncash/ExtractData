#pragma once

#include "../ExtractBase.h"

class COyatu final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool IsSupported(CArcFile* archive);

	bool DecodeSound(CArcFile* archive, const u8* key);
	bool DecodeBGM(CArcFile* archive);
	bool DecodeKOE(CArcFile* archive);
	bool DecodeMSE(CArcFile* archive);
};
