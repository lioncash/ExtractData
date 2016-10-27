#pragma once

class CMajiro final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	struct SRCHeader
	{
		char  szIdentifier[8];
		long  lWidth;
		long  lHeight;
		DWORD dwDataSize;
	};

	bool MountArc1(CArcFile* pclArc);
	bool MountArc2(CArcFile* pclArc);
	bool MountMJO(CArcFile* pclArc);
	bool MountRC(CArcFile* pclArc);

	bool DecodeMJO(CArcFile* pclArc);
	bool DecodeRC(CArcFile* pclArc);

	void read_bits_24(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth);
	void read_bits_8(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth);

	bool AppendMask(CArcFile* pclArc, BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize);
};
