#pragma once

#include "../../ExtractBase.h"

struct SFileInfo;

class CCpz final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	struct SCPZ5Header
	{
		char identifier[4];        // Identifier
		u32 num_dirs;              // Number of directories
		u32 total_dir_index_size;  // Total size of the directory index
		u32 total_file_index_size; // Total size of the file index
		u32 unknown1[4];           // Unknown
		u32 md5[4];                // MD5
		u32 index_key;             // Index Key
		u32 unknown2[3];           // Unknown
	};

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
