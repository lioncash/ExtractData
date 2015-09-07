#pragma once

#include "Krkr.h"

class CTokiPaku final : public CKrkr
{
private:
	BOOL  OnCheckDecrypt(CArcFile* pclArc) override;
	DWORD OnInitDecrypt(CArcFile* pclArc) override;
	DWORD OnDecrypt(BYTE* pvTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey) override;
};
