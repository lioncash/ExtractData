#pragma once

class CNavel final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL MountPac(CArcFile* pclArc);
	BOOL MountWpd(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc) override;

private:
	// WPD Format
	struct FormatWPD
	{
		BYTE    ID[4];      // " DPW"
		DWORD   type;       // Unknown
		DWORD   dummy2;     // Unknown
		DWORD   ChunkByte;  // Number of bytes per chunk
		DWORD   freq;       // Sampling rate
		DWORD   DataSize;   // Data Size
		WORD    FormatID;   // Format ID
		WORD    channels;   // Number of channels
		DWORD   freq2;      // Sampling rate (for some reason there's two)
		DWORD   bps;        // Data Rate (Byte/sec)
		WORD    BlockSize;  // Block Size (Byte/sample * Number of channels)
		WORD    bits;       // Number of bits per sample (bit/sample)
		DWORD   dummy3;     // Unknown
	};
};
