#pragma once

#include "ExtractBase.h"

class CNscr final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountNsa(CArcFile* archive);
	bool MountSar(CArcFile* archive);
	bool MountScr(CArcFile* archive);

	bool DecodeScr(CArcFile* archive);
	bool DecodeNBZ(CArcFile* archive);
	bool DecodeSPB(CArcFile* archive);
	bool DecodeLZSS(CArcFile* archive);

	void GetFileExt(YCString& dst, const u8* buffer);
	u32 GetBit(const u8* src, size_t num_bits_to_read, size_t* num_bytes_read);

	u8 m_mask_for_get_bit = 0;
	u8 m_src_for_get_bit = 0;
};
