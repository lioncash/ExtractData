#pragma once

#include <cstddef>

class CArcFile;

class CWav final
{
public:
	void Init(CArcFile* pclArc, u32 dataSize, u32 freq, u16 channels, u16 bits);
	void Write();
	void Write(u8* buf);
	void Write(u8* buf, size_t size);

private:
	// WAV Format
	struct Header
	{
		u8  riff_id[4]; // "RIFF"
		u32 file_size;  // Filesize
		u8  wave_id[4]; // "WAVE"
		u8  fmt_id[4];  // "fmt "
		u32 chunk_byte; // fmt Number of bytes in chunk
		u16 format_id;  // Format ID
		u16 channels;   // Number of channels
		u32 freq;       // Sampling rate
		u32 bps;        // Data rate (Byte/sec)
		u16 block_size; // Block size (Byte/sample * Number of channels)
		u16 bits;       // Number of bits per sample (bit/sample)
		u8  data_id[4]; // "data"
		u32 data_size;  // Number of bytes of waveform data
	};

	CArcFile* m_archive = nullptr;
	Header m_header{};
};
