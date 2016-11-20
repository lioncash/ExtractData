#pragma once

#include "../ExtractBase.h"

class CVA final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	// NWA Format
	struct NWAHeader
	{
		u16   channels;             // Number of channels
		u16   bits;                 // Number of bits per sample (bit/sample)
		u32   freq;                 // Sampling Rate
		u32   compression_level;    // Compression Level
		u32   dummy1;               // Unknown
		u32   blocks;               // Number of blocks
		u32   data_size;            // Number of bytes of waveform data after decompression
		u32   compressed_file_size; // Compressed file size
		u32   samples;              // Number of decompressed data
		u32   block_size;           // The number of units of data samples of a block when you expand the data
		u32   rest_size;            // Number of sample units of data when you expand the last block
		u32   dummy2;               // Unknown
	};

	// NWK Index Header
	struct IndexNwk
	{
		u32 size;
		u32 start;
		u32 dummy;
	};

	bool MountNwa(CArcFile* archive);
	bool MountNwk(CArcFile* archive);
	bool MountOvk(CArcFile* archive);
	bool DecodeNwa(CArcFile* archive);

	inline int getbits(u8*& data, int& shift, int bits);
};
