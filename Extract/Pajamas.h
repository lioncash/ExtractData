#pragma once

class CPajamas final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	BOOL MountDat1(CArcFile* pclArc);
	BOOL MountDat2(CArcFile* pclArc);

	BOOL DecodeEPA(CArcFile* pclArc);

	BOOL DecompEPA(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, long lWidth);
};
