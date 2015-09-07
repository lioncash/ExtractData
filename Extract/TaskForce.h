#pragma once

#include "../ExtractBase.h"

class CTaskForce final : public CExtractBase
{
public:
	struct SFileEntry
	{
		char  szFileName[256];
		DWORD dwOffset;
		DWORD dwOriginalSize;
		DWORD dwCompressedSize;
	};

	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	BOOL MountDat(CArcFile* pclArc);
	BOOL MountTlz(CArcFile* pclArc);
	BOOL MountBma(CArcFile* pclArc);

	BOOL DecodeTlz(CArcFile* pclArc);
	BOOL DecodeBma(CArcFile* pclArc);
	BOOL DecodeTGA(CArcFile* pclArc);
};
