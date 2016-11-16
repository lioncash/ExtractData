#pragma once

#include "Krkr.h"

class CFateFD final : public CKrkr
{
public:
	bool  OnCheckDecrypt(CArcFile* archive) override;
	DWORD OnInitDecrypt(CArcFile* archive) override;
	DWORD OnDecrypt(BYTE* target, DWORD target_size, DWORD offset, DWORD decrypt_key) override;

private:
	BYTE  m_key[6];
	DWORD m_offset[6];

	// Decode Info
	struct DecodeInfo
	{
		BYTE key;
		DWORD size;
	};
};
