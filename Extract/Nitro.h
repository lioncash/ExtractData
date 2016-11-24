#pragma once

#include "../ExtractBase.h"

class CNitro final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountPak1(CArcFile* archive);
	bool MountPak2(CArcFile* archive);
	bool MountPak3(CArcFile* archive);
	bool MountPak4(CArcFile* archive);
	bool MountPK2(CArcFile* archive);
	bool MountN3Pk(CArcFile* archive);
	bool MountPck(CArcFile* archive);
	bool MountNpp(CArcFile* archive);
	bool MountNpa(CArcFile* archive);

	bool DecodePak1(CArcFile* archive);
	bool DecodePak3(CArcFile* archive);
	bool DecodePak4(CArcFile* archive);
	bool DecodePK2(CArcFile* archive);
	bool DecodeN3Pk(CArcFile* archive);
	bool DecodeNpa(CArcFile* archive);

	void DecryptPak3(u8* data, size_t size, size_t offset, const SFileInfo* file_info);
	void DecryptPak4(u8* data, size_t size, size_t offset, const SFileInfo* file_info);
	void DecryptN3Pk(u8* data, size_t size, size_t offset, const SFileInfo* file_info, u8& key);
	void DecryptNpa(u8* data, size_t size, size_t offset, const SFileInfo* file_info);
};
