#pragma once

#include "../ExtractBase.h"

class CVA final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	// NWA Format
	struct NWAHed
	{
		WORD    channels;       // Number of channels
		WORD    bits;           // Number of bits per sample (bit/sample)
		DWORD   freq;           // Sampling Rate
		DWORD   CmpLevel;       // Compression Level
		DWORD   dummy1;         // Unknown
		DWORD   blocks;         // Number of blocks
		DWORD   DataSize;       // Number of bytes of waveform data after decompression
		DWORD   CompFileSize;   // Compressed file size
		DWORD   samples;        // Number of decompressed data
		DWORD   BlockSize;      // The number of units of data samples of a block when you expand the data
		DWORD   RestSize;       // Number of sample units of data when you expand the last block
		DWORD   dummy2;         // Unknown
	};

	// NWK Index Header
	struct IndexNwk
	{
		DWORD   size;
		DWORD   start;
		DWORD   dummy;
	};

	bool MountNwa(CArcFile* pclArc);
	bool MountNwk(CArcFile* pclArc);
	bool MountOvk(CArcFile* pclArc);
	bool DecodeNwa(CArcFile* pclArc);

	inline int getbits(LPBYTE& data, int& shift, int bits);
};
