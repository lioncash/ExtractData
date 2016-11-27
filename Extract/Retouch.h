#pragma once

#include "ExtractBase.h"

class CRetouch final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	struct GYUHeader
	{
		u8  identifier[4];
		u32 unknown;
		u32 key;
		u32 bpp;
		s32 width;
		s32 height;
		u32 compressed_sizes[2];
		u32 pallets;
	};

	bool MountGYU(CArcFile* archive);

	bool DecodeGYU(CArcFile* archive);

	bool DecryptGYU(u8* src, size_t src_size, u32 key);

	bool DecompGYU(u8* dst, size_t dst_size, const u8* src, size_t src_size);
};
