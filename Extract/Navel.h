#pragma once

#include "ExtractBase.h"

class CNavel final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	// WPD Format
	struct FormatWPD
	{
		u8  id[4];       // " DPW"
		u32 type;        // Unknown
		u32 dummy2;      // Unknown
		u32 chunk_byte;  // Number of bytes per chunk
		u32 freq;        // Sampling rate
		u32 data_size;   // Data Size
		u16 format_id;   // Format ID
		u16 channels;    // Number of channels
		u32 freq2;       // Sampling rate (for some reason there's two)
		u32 bps;         // Data Rate (Byte/sec)
		u16 block_size;  // Block Size (Byte/sample * Number of channels)
		u16 bits;        // Number of bits per sample (bit/sample)
		u32 dummy3;      // Unknown
	};

	bool MountPac(CArcFile* archive);
	bool MountWpd(CArcFile* archive);
};
