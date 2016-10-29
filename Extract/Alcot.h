#pragma once

#include "../ExtractBase.h"

class CAlcot final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	bool DecodeASB(CArcFile* pclArc);
	bool DecodeCPB(CArcFile* pclArc);

	void Decomp(LPBYTE dst, DWORD dstSize, LPBYTE src);
	void Decrypt(LPBYTE src, DWORD srcSize, DWORD dstSize);
};
