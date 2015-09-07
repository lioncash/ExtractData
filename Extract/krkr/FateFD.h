#pragma once

#include "Krkr.h"

class CFateFD final : public CKrkr
{
private:
	BOOL  OnCheckDecrypt(CArcFile* pclArc) override;
	DWORD OnInitDecrypt(CArcFile* pclArc) override;
	DWORD OnDecrypt(BYTE* pvTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey) override;

	BYTE  m_abtKey[6];
	DWORD m_adwOffset[6];

	// Decode Info
	struct DecodeInfo
	{
		BYTE key;
		DWORD size;
	};
};
