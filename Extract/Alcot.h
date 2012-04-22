#pragma once

class CAlcot : public CExtractBase {
public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);
	BOOL DecodeASB(CArcFile* pclArc);
	BOOL DecodeCPB(CArcFile* pclArc);

	void Decomp(LPBYTE dst, DWORD dstSize, LPBYTE src);
	void Decrypt(LPBYTE src, DWORD srcSize, DWORD dstSize);
};
