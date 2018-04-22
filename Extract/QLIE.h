#pragma once

#include "ExtractBase.h"

class CQLIE final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	enum
	{
		ABIMGDAT10 = 0x00000000,
		ABIMGDAT11,
		ABSNDDAT10,
		ABSNDDAT11,
	};

	struct FileNameInfo
	{
		YCString file_name; // Filename
		u32      count;     // Number of times
	};

	bool DecodeB(CArcFile* archive, u8* src, u32 src_size);
	bool DecodeABMP7(CArcFile* archive, u8* src, u32 src_size);
	bool DecodeABMP10(CArcFile* archive, u8* src, u32 src_size, u32* src_index_ptr = nullptr, const YCString& b_file_name = _T(""));

	void Decomp(u8* dst, u32 dst_size, const u8* src, u32 src_size);

	static void DecryptFileName(u8* in, u32 size, u32 seed);
	static void Decrypt(u8* buf, u32 buf_len, u32 seed);

	u64 padw(u64 a, u64 b);
	u32 crc_or_something(const u8* buff, size_t len);
	static void DecryptFileNameV3(u8* buff, u32 len, u32 seed);
	static void DecryptV3(u8* buff, u32 len, u32 seed);

	YCString GetExtension(const u8* src);

	void EraseNotUsePathWord(YCString& path);
};
