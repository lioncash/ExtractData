#pragma once

#include "Krkr.h"

class CNatsupochi final : public CKrkr
{
public:
	bool  OnCheckDecrypt(CArcFile* archive) override;
	DWORD OnInitDecrypt(CArcFile* archive) override;
	DWORD OnDecrypt(BYTE* target, DWORD target_size, DWORD offset, DWORD decrypt_key) override;
};
