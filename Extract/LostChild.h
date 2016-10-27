#pragma once

class CLostChild final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;
	bool Extract(CArcFile* pclArc) override;

private:
	bool DecodeESUR(CArcFile* pclArc);
	bool DecodeLAD(CArcFile* pclArc);

	bool DecompLZSS(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, DWORD dwDicSize, DWORD dwDicPtr, DWORD dwLengthOffset);
};
