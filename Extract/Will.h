#pragma once

#include "../ExtractBase.h"

class CWill final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	void DecompLZSS(u8* dst, size_t dst_size, const u8* src, size_t src_size);

	bool AppendMask(u8* dst, size_t dst_size, const u8* src, size_t src_size, const u8* mask, size_t mask_size);
};
