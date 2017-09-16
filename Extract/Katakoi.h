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

	bool GetNameIndex(CArcFile* archive, std::vector<BYTE>& sec, DWORD& name_index);
	bool GetPathToSec(LPTSTR sec_path, const YCString& archive_path);

	void GetBit(LPBYTE& src, DWORD& flags);
	bool DecompImage(LPBYTE dst, DWORD dst_size, LPBYTE src, DWORD src_size);

	bool Compose(LPBYTE dst, DWORD dst_size, LPBYTE src, DWORD src_size, long dst_width, long src_width, WORD bpp);
	bool DecodeCompose(CArcFile* archive, LPBYTE diff, DWORD diff_size, long diff_width, long diff_height, WORD diff_bpp);
};
