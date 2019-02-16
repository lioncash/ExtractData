#pragma once

#include "../../ExtractBase.h"

struct SFileInfo;

class CCpz final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountCpz1(CArcFile* archive);
	bool MountCpz2(CArcFile* archive);
	bool MountCpz3(CArcFile* archive);
	bool MountCpz5(CArcFile* archive);

	bool DecodeCpz1(CArcFile* archive);
	bool DecodeCpz2(CArcFile* archive);
	bool DecodeCpz3(CArcFile* archive);
	bool DecodeCpz5(CArcFile* archive);

	static void Decrypt1(u8* target, size_t size);
	static void Decrypt2(u8* target, size_t size, u32 key);
	static void Decrypt3(u8* target, size_t size, u32 key);
	static void Decrypt5(u8* target, size_t size, u32 key);

	static const u8* InitDecryptWithTable5(u32 key, u32 seed);
	static void DecryptWithTable5(u8* target, size_t size, const u8* decryption_table, u32 key);
	static void DecryptOfData5(u8* target, size_t size, const u8* decryption_table, const u32* key, u32 seed);

	// Callback function from PB3B
	static void OnDecrypt3FromPB3B(u8* target, size_t target_size, CArcFile* archive, const SFileInfo& file_info);
	static void OnDecrypt5FromPB3B(u8* target, size_t target_size, CArcFile* archive, const SFileInfo& file_info);
};
