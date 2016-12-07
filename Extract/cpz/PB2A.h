#pragma once

#include "Extract/cpz/PB.h"

class CArcFile;

class CPB2A final : public CPB
{
public:
	bool Decode(CArcFile* archive, u8* src, size_t src_size);

private:
	void Decrypt(u8* target, size_t size);

	bool Decode1(CArcFile* archive, const u8* src, size_t src_size, long width, long height, u16 bpp);
	bool Decode2(CArcFile* archive, const u8* src, size_t src_size, long width, long height, u16 bpp);
	bool Decode4(CArcFile* archive, const u8* src, size_t src_size, long width, long height, u16 bpp);
	bool Decode5(CArcFile* archive, const u8* src, size_t src_size, long width, long height, u16 bpp);
	bool Decode6(CArcFile* archive, const u8* src, size_t src_size, long width, long height, u16 bpp);

	bool Decomp1(u8* dst, size_t dst_size, const u8* src, size_t src_size, long width, long height, u16 bpp);
	bool Decomp2(u8* dst, size_t dst_size, const u8* src, size_t src_size, long width, long height, u16 bpp);
	bool Decomp4(u8* dst, size_t dst_size, const u8* src, size_t src_size, long width, long height, u16 bpp);
	bool Decomp5(u8* dst, size_t dst_size, const u8* src, size_t src_size, long width, long height, u16 bpp, const u8* base, u32 frame_number);
	bool Decomp6(u8* dst, size_t dst_size, const u8* src, size_t src_size, long width, long height, u16 bpp, const u8* base, u32 frame_number);
};
