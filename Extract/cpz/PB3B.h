#pragma once

#include "Extract/cpz/PB.h"

class CArcFile;
struct SFileInfo;

class CPB3B final : public CPB
{
public:
	using PB3B_DECRYPT = void (*)(u8*, size_t, CArcFile*, const SFileInfo&);

	bool Decode(CArcFile* archive, u8* src, size_t src_size, PB3B_DECRYPT decryption_function);

private:
	void Decrypt(u8* target, size_t size);

	bool Decode1(CArcFile* archive, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp);
	bool Decode3(CArcFile* archive, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp);
	bool Decode4(CArcFile* archive, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp);
	bool Decode5(CArcFile* archive, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp);
	bool Decode6(CArcFile* archive, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp, PB3B_DECRYPT decryption_function);

	bool Decomp1(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp);
	bool Decomp3(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp);
	bool Decomp4(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp);
	bool Decomp5(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp);
	bool Decomp6(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp);
};
