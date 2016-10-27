#pragma once

class CCyc final : public CExtractBase
{
public:
	// Mount
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	// References gtb, Mount gpk (dwq has been archived)
	bool MountGpk(CArcFile* pclArc);
	// References vtb, Mount vpk (wgq has been archived)
	bool MountVpk(CArcFile* pclArc);
	// Mount dwq (BMP: PACKTYPE=0, PACKBMP+MASK: PACKTYPE=3A, JPEG: PACKTYPE=5, JPEG+MASK: PACKTYPE=7A)
	bool MountDwq(CArcFile* pclArc);
	// Mount wgq (OGG: PACKTYPE=6)
	bool MountWgq(CArcFile* pclArc);
	// Mount vaw (WAV: PACKTYPE=0, RIFF OGG: PACKTYPE=2)
	bool MountVaw(CArcFile* pclArc);
	// Mount xtx (Text Data)
	bool MountXtx(CArcFile* pclArc);
	// Mount fxf (Inverted data bit xtx)
	bool MountFxf(CArcFile* pclArc);

	// Decode dwq
	bool DecodeDwq(CArcFile* pclArc);
	// Decode wgq
	bool DecodeWgq(CArcFile* pclArc);
	// Decode vaw
	bool DecodeVaw(CArcFile* pclArc);
	// Decode xtx
	bool DecodeXtx(CArcFile* pclArc);
	// Decode fxf
	bool DecodeFxf(CArcFile* pclArc);

	// RLE decompression to dst
	void DecompRLE(LPBYTE dst, LPBYTE src, LONG width, LONG height);
	// RGB to BGR
	void RGBtoBGR(LPBYTE buf, LONG widht, LONG height);
};
