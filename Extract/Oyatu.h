#pragma once

#include "../ExtractBase.h"

class COyatu final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	bool IsSupported(CArcFile* pclArc);

	bool DecodeSound(CArcFile* pclArc, const BYTE* pbtKey);
	bool DecodeBGM(CArcFile* pclArc);
	bool DecodeKOE(CArcFile* pclArc);
	bool DecodeMSE(CArcFile* pclArc);
};
