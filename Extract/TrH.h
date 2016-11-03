#pragma once

#include "../ExtractBase.h"

class CTrH final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	// PX Format
	struct PXHed
	{
		u8  px_id[4];       // "fPX "
		u32 file_size;      // File Size
		u8  ctrk_id[4];     // "cTRK"
		u32 data_size;      // Number of bytes of waveform data
		u32 dummy1;         // Unknown
		u32 dummy2;         // Unknown
		u32 dummy3;         // Unknown
		u32 freq;           // Sampling Rate
		u16 dummy4;         // Unknown
		u16 channels;       // Number of channels
		u32 chunk_byte;     // Numer of bytes per chunk
		u16 bits;           // Number of bits per sample(bit/sample)
		u16 format_id;      // Format ID
	};
};
