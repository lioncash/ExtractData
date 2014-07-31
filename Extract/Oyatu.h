#pragma once

#include "../ExtractBase.h"

class COyatu : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);


protected:
	BOOL IsSupported(CArcFile* pclArc);

	BOOL DecodeSound(CArcFile* pclArc, const BYTE* pbtKey);
	BOOL DecodeBGM(CArcFile* pclArc);
	BOOL DecodeKOE(CArcFile* pclArc);
	BOOL DecodeMSE(CArcFile* pclArc);
};
