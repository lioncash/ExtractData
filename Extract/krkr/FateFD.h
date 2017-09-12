#pragma once

#include "Extract/krkr/Krkr.h"

class CFateFD final : public CKrkr
{
public:
	bool OnCheckDecrypt(CArcFile* archive) override;
	u32 OnInitDecrypt(CArcFile* archive) override;
	size_t OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key) override;

private:
	std::array<u8, 6> m_key;
	std::array<size_t, 6> m_offset;

	// Decode Info
	struct DecodeInfo
	{
		u8 key;
		size_t size;
	};
};
