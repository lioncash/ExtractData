#pragma once

#include "ExtractBase.h"

class CKatakoi final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountIar(CArcFile* archive);
	bool MountWar(CArcFile* archive);

	bool DecodeIar(CArcFile* archive);
	bool DecodeWar(CArcFile* archive);

	bool GetNameIndex(CArcFile* archive, std::vector<u8>& sec, u32& name_index);
	bool GetPathToSec(LPTSTR sec_path, const YCString& archive_path);

	void GetBit(u8*& src, u32& flags);
	bool DecompImage(u8* dst, size_t dst_size, u8* src, size_t src_size);

	bool Compose(u8* dst, size_t dst_size, u8* src, size_t src_size, long dst_width, long src_width, u16 bpp);
	bool DecodeCompose(CArcFile* archive, u8* diff, size_t diff_size, long diff_width, long diff_height, u16 diff_bpp);
};
