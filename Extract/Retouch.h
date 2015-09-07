#pragma	once

#include "../ExtractBase.h"

class CRetouch final : public CExtractBase
{
public:
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

	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	BOOL MountGYU(CArcFile* pclArc);

	BOOL DecodeGYU(CArcFile* pclArc);

	BOOL DecryptGYU(void* pvSrc, DWORD dwSrcSize, DWORD dwKey);

	BOOL DecompGYU(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
};
