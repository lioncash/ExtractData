#pragma once

#include "../ExtractBase.h"

class CCircus final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	struct SPCMHeader
	{
		char  szIdentifier[4]; // Identifier
		DWORD dwDataSize;      // Data Size
		DWORD dwFlags;         // Flags
		WORD  wFormatID;       // Format ID
		WORD  wChannels;       // Number of channels
		DWORD dwFreq;          // Sampling rate
		DWORD dwBps;           // Data Rate
		WORD  wBlockSize;      // Block size
		WORD  wBits;           // Number of bits
	};

	bool MountPCK(CArcFile* pclArc);
	bool MountVoiceDat(CArcFile* pclArc);
	bool MountCRX(CArcFile* pclArc);
	bool MountPCM(CArcFile* pclArc);

	bool DecodeCRX(CArcFile* pclArc);
	bool DecodeCRX1(CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize);
	bool DecodeCRX2(CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize);
	bool DecodePCM(CArcFile* pclArc);
	bool DecodePCM1(CArcFile* pclArc, const SPCMHeader& rfstPCMHeader);
	bool DecodePCM2(CArcFile* pclArc, const SPCMHeader& rfstPCMHeader);

	bool DecompLZSS(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
	bool DecompCRX2(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp, WORD wFlags);
	bool DecompPCM1(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
	bool DecompPCM2(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
};
