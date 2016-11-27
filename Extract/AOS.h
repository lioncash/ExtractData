#pragma once

#include "ExtractBase.h"

class CAOS final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	struct SABMHeader
	{
		BITMAPFILEHEADER bmp_file_header; // Bitmap File Header
		BITMAPINFOHEADER bmp_info_header; // Bitmap Info Header
		u16              unknown[2];      // Unknown
		u32              frames;          // Number of frames
	};

	bool DecodeABM(CArcFile* archive);
	bool DecodeMask(CArcFile* archive);
	bool DecodeScript(CArcFile* archive);

	bool DecompABM(u8* dst, size_t dst_size, const u8* src, size_t src_size);
	bool DecompScript(u8* dst, size_t dst_size, const u8* src, size_t src_size);

	u32 CreateHuffmanTable(u32* bit0_table, u32* bit1_table, const u8* src, size_t* src_ptr, size_t* table_ptr, u32* current_src, u32* bit_shift);
	bool DecompHuffman(u8* dst, size_t dst_size, const u32* bit0_table, const u32* bit1_table, const u8* src, size_t dwRoot, u32 current_src, u32 bit_shift);
};
