#pragma once

#include "Krkr.h"

class CHimesyo final : public CKrkr
{
public:
	bool  OnCheckDecrypt(CArcFile* archive) override;
	DWORD OnInitDecrypt(CArcFile* archive) override;
	DWORD OnDecrypt(BYTE* target, DWORD target_size, DWORD offset, DWORD decrypt_key) override;

private:
	DWORD m_change_decrypt_key;
};
