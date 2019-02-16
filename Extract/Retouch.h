#pragma once

#include "ExtractBase.h"

class CRetouch final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountGYU(CArcFile* archive);

	bool DecodeGYU(CArcFile* archive);

	bool DecryptGYU(u8* src, size_t src_size, u32 key);

	bool DecompGYU(u8* dst, size_t dst_size, const u8* src, size_t src_size);
};
