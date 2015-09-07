#pragma once

#include "../ExtractBase.h"

class COyatu final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	BOOL IsSupported(CArcFile* pclArc);

	BOOL DecodeSound(CArcFile* pclArc, const BYTE* pbtKey);
	BOOL DecodeBGM(CArcFile* pclArc);
	BOOL DecodeKOE(CArcFile* pclArc);
	BOOL DecodeMSE(CArcFile* pclArc);
};
