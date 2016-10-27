#pragma	once

#include "../ExtractBase.h"

class CRetouch final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	struct SGYUHeader
	{
		BYTE  abtIdentifier[4];
		DWORD dwUnknown;
		DWORD dwKey;
		DWORD dwBpp;
		long  lWidth;
		long  lHeight;
		DWORD adwCompSize[2];
		DWORD dwPallets;
	};

	bool MountGYU(CArcFile* pclArc);

	bool DecodeGYU(CArcFile* pclArc);

	bool DecryptGYU(void* pvSrc, DWORD dwSrcSize, DWORD dwKey);

	bool DecompGYU(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
};
