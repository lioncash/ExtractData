#pragma once

class CLostChild final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;
	BOOL Extract(CArcFile* pclArc) override;

private:
	bool DecodeESUR(CArcFile* pclArc);
	bool DecodeLAD(CArcFile* pclArc);

	bool DecompLZSS(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, DWORD dwDicSize, DWORD dwDicPtr, DWORD dwLengthOffset);
};
