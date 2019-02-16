#pragma once

#include "ExtractBase.h"

class CMajiro final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountArc1(CArcFile* archive);
	bool MountArc2(CArcFile* archive);
	bool MountMJO(CArcFile* archive);
	bool MountRC(CArcFile* archive);

	bool DecodeMJO(CArcFile* archive);
	bool DecodeRC(CArcFile* archive);

	void read_bits_24(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width);
	void read_bits_8(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width);

	bool AppendMask(CArcFile* archive, u8* dst, size_t dst_size, const u8* src, size_t src_size);
};
