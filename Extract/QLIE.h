#pragma once

#include "ExtractBase.h"

class CQLIE final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	enum
	{
		ABIMGDAT10 = 0x00000000,
		ABIMGDAT11,
		ABSNDDAT10,
		ABSNDDAT11,
	};

	struct SFileNameInfo
	{
		YCString clsFileName; // Filename
		u32      dwCount;     // Number of times
	};

	bool DecodeB(CArcFile* pclArc, u8* pbtSrc, u32 dwSrcSize);
	bool DecodeABMP7(CArcFile* pclArc, u8* pbtSrc, u32 dwSrcSize, u32* pdwSrcIndex = nullptr, const YCString& rfclsBFileName = _T(""));
	bool DecodeABMP10(CArcFile* pclArc, u8* pbtSrc, u32 dwSrcSize, u32* pdwSrcIndex = nullptr, const YCString& rfclsBFileName = _T(""));

	void Decomp(u8* dst, u32 dstSize, const u8* src, u32 srcSize);

	static void DecryptFileName(u8* in, u32 size, u32 seed);
	static void Decrypt(u8* buf, u32 buf_len, u32 seed);

	u64 padw(u64 a, u64 b);
	u32 crc_or_something(const u8* buff, size_t len);
	static void DecryptFileNameV3(u8* buff, u32 len, u32 seed);
	static void DecryptV3(u8* buff, u32 len, u32 seed);

	YCString GetExtension(const u8* src);

	void EraseNotUsePathWord(YCString& clsPath);
};
