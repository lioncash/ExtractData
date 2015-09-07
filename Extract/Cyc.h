#pragma once

class CCyc final : public CExtractBase
{
public:
	// Mount
	BOOL Mount(CArcFile* pclArc) override;
	// References gtb, Mount gpk (dwq has been archived)
	BOOL MountGpk(CArcFile* pclArc);
	// References vtb, Mount vpk (wgq has been archived)
	BOOL MountVpk(CArcFile* pclArc);
	// Mount dwq (BMP: PACKTYPE=0, PACKBMP+MASK: PACKTYPE=3A, JPEG: PACKTYPE=5, JPEG+MASK: PACKTYPE=7A)
	BOOL MountDwq(CArcFile* pclArc);
	// Mount wgq (OGG: PACKTYPE=6)
	BOOL MountWgq(CArcFile* pclArc);
	// Mount vaw (WAV: PACKTYPE=0, RIFF OGG: PACKTYPE=2)
	BOOL MountVaw(CArcFile* pclArc);
	// Mount xtx (Text Data)
	BOOL MountXtx(CArcFile* pclArc);
	// Mount fxf (Inverted data bit xtx)
	BOOL MountFxf(CArcFile* pclArc);

	// Decode
	BOOL Decode(CArcFile* pclArc);
	// Decode dwq
	BOOL DecodeDwq(CArcFile* pclArc);
	// Decode wgq
	BOOL DecodeWgq(CArcFile* pclArc);
	// Decode vaw
	BOOL DecodeVaw(CArcFile* pclArc);
	// Decode xtx
	BOOL DecodeXtx(CArcFile* pclArc);
	// Decode fxf
	BOOL DecodeFxf(CArcFile* pclArc);

	// RLE decompression to dst
	void DecompRLE(LPBYTE dst, LPBYTE src, LONG width, LONG height);
	// RGB to BGR
	void RGBtoBGR(LPBYTE buf, LONG widht, LONG height);
};
