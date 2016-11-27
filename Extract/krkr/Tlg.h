#pragma once

#include "ExtractBase.h"

class CTlg final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;
	bool Decode(CArcFile* archive, u8* src);

private:
	bool DecompTLG5(CArcFile* archive, const u8* src);
	bool DecompTLG6(CArcFile* archive, const u8* src);
	u32 DecompLZSS(u8* dst, const u8* src, size_t src_size, u8* dictionary, u32 initial_r);
	void ComposeColors(u8* dst, const u8* upper, const u8* const* buffer, u32 width, u32 colors);

	void TVPTLG6DecodeGolombValues(u8* pixelbuf, u32 pixel_count, const u8* bit_pool, u8 color);

	void TVPTLG6DecodeLineGeneric(const u32* prev_line, u32* current_line, u32 width,
	                              u32 start_block, u32 block_limit, const u8* filter_types,
	                              u32 skip_block_bytes, const u32* in, u32 initial_p,
	                              u32 odd_skip, u32 dir);
	
	void TVPTLG6DecodeLine(const u32* prev_line, u32* current_line, u32 width,
	                       u32 block_count, const u8* filter_types, u32 skip_block_bytes,
	                       const u32* in, u32 initial_p, u32 odd_skip,
	                       u32 dir);
	
	u32 make_gt_mask(u32 a, u32 b);
	u32 packed_bytes_add(u32 a, u32 b);
	u32 med2(u32 a, u32 b, u32 c);
	u32 med(u32 a, u32 b, u32 c, u32 v);
	u32 avg(u32 a, u32 b, u32 c, u32 v);

	void InitTLG6Table();
};
