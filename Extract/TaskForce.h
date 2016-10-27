#pragma once

#include "../ExtractBase.h"

class CTaskForce final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	struct SFileEntry
	{
		char  szFileName[256];
		DWORD dwOffset;
		DWORD dwOriginalSize;
		DWORD dwCompressedSize;
	};

	bool MountDat(CArcFile* pclArc);
	bool MountTlz(CArcFile* pclArc);
	bool MountBma(CArcFile* pclArc);

	bool DecodeTlz(CArcFile* pclArc);
	bool DecodeBma(CArcFile* pclArc);
	bool DecodeTGA(CArcFile* pclArc);
};
