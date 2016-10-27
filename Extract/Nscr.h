#pragma once

class CNscr final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	bool MountNsa(CArcFile* pclArc);
	bool MountSar(CArcFile* pclArc);
	bool MountScr(CArcFile* pclArc);

	bool DecodeScr(CArcFile* pclArc);
	bool DecodeNBZ(CArcFile* pclArc);
	bool DecodeSPB(CArcFile* pclArc);
	bool DecodeLZSS(CArcFile* pclArc);

	void  GetFileExt(YCString& clsDst, const BYTE* pbtBuffer);
	DWORD GetBit(const BYTE* pbtSrc, DWORD dwReadBitLength, DWORD* pdwReadByteLength);

	BYTE btMaskForGetBit;
	BYTE btSrcForGetBit;
};
