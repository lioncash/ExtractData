#pragma once

#include "ExtractBase.h"

class CCircusPak final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	using DecryptionFunction = void(*)(u8*, size_t, const u8*);

	struct PakFileInfoType1
	{
		char file_name[24];
		u32  offset;
		u32  compressed_file_size;
	};

	struct PakFileInfoType2
	{
		char file_name[32];
		u32  offset;
		u32  compressed_file_size;
	};

	struct PakFileInfoType3
	{
		wchar_t file_name[32];
		u32     file_size;
		u32     compressed_file_size;
		u32     offset;
	};

	bool MountPakForKujiraCons(CArcFile* archive);
	bool MountPakForKujira(CArcFile* archive);
	bool MountPakForACDC(CArcFile* archive);
	bool MountPakForDCGS(CArcFile* archive);

	bool DecodePakForKujiraCons(CArcFile* archive);
	bool DecodePakForKujira(CArcFile* archive);
	bool DecodePakForACDC(CArcFile* archive);
	bool DecodePakForDCGS(CArcFile* archive);

	bool DecodeBMP(CArcFile* archive, const u8* src, size_t src_size);
	bool DecodeCps(CArcFile* archive, const u8* src, size_t src_size);
	bool DecodeEtc(CArcFile* archive, DecryptionFunction decryption_function, const u8* key);

	bool DecompCCC0(u8* dst, size_t dst_size, const u8* src, size_t src_size);
	bool DecompCCM0(u8* dst, size_t dst_size, const u8* src, size_t src_size);

	u32 GetBit(const u8* src, size_t* src_bit_idx, u32 num_bits_to_read);

	static void Decrypt1(u8* target, size_t target_size, const u8* key);
	static void Decrypt2(u8* target, size_t target_size, const u8* key);
};
