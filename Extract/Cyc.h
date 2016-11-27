#pragma once

#include "ExtractBase.h"

class CCyc final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	// References gtb, Mount gpk (dwq has been archived)
	bool MountGpk(CArcFile* archive);
	// References vtb, Mount vpk (wgq has been archived)
	bool MountVpk(CArcFile* archive);
	// Mount dwq (BMP: PACKTYPE=0, PACKBMP+MASK: PACKTYPE=3A, JPEG: PACKTYPE=5, JPEG+MASK: PACKTYPE=7A)
	bool MountDwq(CArcFile* archive);
	// Mount wgq (OGG: PACKTYPE=6)
	bool MountWgq(CArcFile* archive);
	// Mount vaw (WAV: PACKTYPE=0, RIFF OGG: PACKTYPE=2)
	bool MountVaw(CArcFile* archive);
	// Mount xtx (Text Data)
	bool MountXtx(CArcFile* archive);
	// Mount fxf (Inverted data bit xtx)
	bool MountFxf(CArcFile* archive);

	// Decode dwq
	bool DecodeDwq(CArcFile* archive);
	// Decode wgq
	bool DecodeWgq(CArcFile* archive);
	// Decode vaw
	bool DecodeVaw(CArcFile* archive);
	// Decode xtx
	bool DecodeXtx(CArcFile* archive);
	// Decode fxf
	bool DecodeFxf(CArcFile* archive);

	// RLE decompression to dst
	void DecompRLE(u8* dst, const u8* src, s32 width, s32 height);
	// RGB to BGR
	void RGBtoBGR(u8* buf, s32 width, s32 height);
};
