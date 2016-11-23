#pragma once

#include "../ExtractBase.h"

class CHimauri final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountHim4(CArcFile* archive);
	bool MountHim5(CArcFile* archive);

	void Decomp(u8* dst, size_t dst_size, const u8* src, size_t src_size);
	bool Compose(u8* dst, size_t dst_size, const u8* base, size_t base_size, const u8* diff, size_t diff_size);
};
