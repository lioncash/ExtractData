#pragma once

class CMajiro final : public CExtractBase
{
public:
	struct SRCHeader
	{
		char  szIdentifier[8];
		long  lWidth;
		long  lHeight;
		DWORD dwDataSize;
	};

	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	BOOL MountArc1(CArcFile* pclArc);
	BOOL MountArc2(CArcFile* pclArc);
	BOOL MountMJO(CArcFile* pclArc);
	BOOL MountRC(CArcFile* pclArc);

	BOOL DecodeMJO(CArcFile* pclArc);
	BOOL DecodeRC(CArcFile* pclArc);

	void read_bits_24(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth);
	void read_bits_8(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth);

	BOOL AppendMask(CArcFile* pclArc, BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize);
};
