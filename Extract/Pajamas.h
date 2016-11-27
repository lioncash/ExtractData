#pragma once

#include "ExtractBase.h"

class CPajamas final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountDat1(CArcFile* archive);
	bool MountDat2(CArcFile* archive);

	bool DecodeEPA(CArcFile* archive);
	void DecompEPA(u8* dst, size_t dst_size, const u8* src, size_t src_size, u32 width);
};
