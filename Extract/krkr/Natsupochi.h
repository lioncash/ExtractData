#pragma once

#include "Krkr.h"

class CNatsupochi final : public CKrkr
{
public:
	bool  OnCheckDecrypt(CArcFile* archive) override;
	u32 OnInitDecrypt(CArcFile* archive) override;
	size_t OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key) override;
};
