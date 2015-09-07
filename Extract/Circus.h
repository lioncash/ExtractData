#pragma once

#include "../ExtractBase.h"

class CCircus final : public CExtractBase
{
public:
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

	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	BOOL MountPCK(CArcFile* pclArc);
	BOOL MountVoiceDat(CArcFile* pclArc);
	BOOL MountCRX(CArcFile* pclArc);
	BOOL MountPCM(CArcFile* pclArc);

	BOOL DecodeCRX(CArcFile* pclArc);
	BOOL DecodeCRX1(CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize);
	BOOL DecodeCRX2(CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize);
	BOOL DecodePCM(CArcFile* pclArc);
	BOOL DecodePCM1(CArcFile* pclArc, const SPCMHeader& rfstPCMHeader);
	BOOL DecodePCM2(CArcFile* pclArc, const SPCMHeader& rfstPCMHeader);

	BOOL DecompLZSS(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
	BOOL DecompCRX2(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp, WORD wFlags);
	BOOL DecompPCM1(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
	BOOL DecompPCM2(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
};
