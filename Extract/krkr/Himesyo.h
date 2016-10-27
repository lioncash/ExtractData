#pragma once

#include "Krkr.h"

class CHimesyo final : public CKrkr
{
public:
	BOOL  OnCheckDecrypt(CArcFile* pclArc) override;
	DWORD OnInitDecrypt(CArcFile* pclArc) override;
	DWORD OnDecrypt(BYTE* pvTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey) override;

private:
	DWORD m_dwChangeDecryptKey;
};
