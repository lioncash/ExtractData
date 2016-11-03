#pragma	once

#include "../ExtractBase.h"

class CRetouch final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

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

	bool DecompGYU(u8* dst, size_t dst_size, const u8* src, size_t src_size);
};
