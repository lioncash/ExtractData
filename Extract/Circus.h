#pragma once

#include "ExtractBase.h"

class CCircus final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
  struct SPCMHeader;

	bool MountPCK(CArcFile* archive);
	bool MountVoiceDat(CArcFile* archive);
	bool MountCRX(CArcFile* archive);
	bool MountPCM(CArcFile* archive);

	bool DecodeCRX(CArcFile* archive);
	bool DecodeCRX1(CArcFile* archive, const u8* src, size_t src_size);
	bool DecodeCRX2(CArcFile* archive, const u8* src, size_t src_size);
	bool DecodePCM(CArcFile* archive);
	bool DecodePCM1(CArcFile* archive, const SPCMHeader& pcm_header);
	bool DecodePCM2(CArcFile* archive, const SPCMHeader& pcm_header);

	bool DecompLZSS(u8* dst, size_t dst_size, const u8* src, size_t src_size);
	bool DecompCRX2(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp, u16 flags);
	bool DecompPCM1(u8* dst, size_t dst_size, const u8* src, size_t src_size);
	bool DecompPCM2(u8* dst, size_t dst_size, const u8* src, size_t src_size);
};
