#pragma once

class CAlcot final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;
	BOOL DecodeASB(CArcFile* pclArc);
	BOOL DecodeCPB(CArcFile* pclArc);

	void Decomp(LPBYTE dst, DWORD dstSize, LPBYTE src);
	void Decrypt(LPBYTE src, DWORD srcSize, DWORD dstSize);
};
