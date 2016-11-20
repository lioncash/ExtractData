#pragma once

#include "../ExtractBase.h"

class CLostChild final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;
	bool Extract(CArcFile* archive) override;

private:
	bool DecodeESUR(CArcFile* archive);
	bool DecodeLAD(CArcFile* archive);

	bool DecompLZSS(u8* dst, size_t dst_size, const u8* src, size_t src_size, size_t dic_size, size_t dic_ptr, size_t length_offset);
};
