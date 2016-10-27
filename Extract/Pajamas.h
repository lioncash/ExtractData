#pragma once

class CPajamas final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	bool MountDat1(CArcFile* pclArc);
	bool MountDat2(CArcFile* pclArc);

	bool DecodeEPA(CArcFile* pclArc);

	bool DecompEPA(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, long lWidth);
};
