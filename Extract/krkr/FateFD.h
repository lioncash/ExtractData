#pragma once

#include "Extract/krkr/Krkr.h"

class CFateFD final : public CKrkr
{
public:
	bool OnCheckDecrypt(CArcFile* archive) override;
	u32 OnInitDecrypt(CArcFile* archive) override;
	size_t OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key) override;

private:
	u8  m_key[6];
	size_t m_offset[6];

	// Decode Info
	struct DecodeInfo
	{
		u8 key;
		size_t size;
	};
};
