#pragma once

class CTrH final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	// PX Format
	struct PXHed
	{
		BYTE    pxID[4];        // "fPX "
		DWORD   FileSize;       // File Size
		BYTE    CtrkID[4];      // "cTRK"
		DWORD   DataSize;       // Number of bytes of waveform data
		DWORD   dummy1;         // Unknown
		DWORD   dummy2;         // Unknown
		DWORD   dummy3;         // Unknown
		DWORD   freq;           // Sampling Rate
		WORD    dummy4;         // Unknown
		WORD    channels;       // Number of channels
		DWORD   ChunkByte;      // Numer of bytes per chunk
		WORD    bits;           // Number of bits per sample(bit/sample)
		WORD    FormatID;       // Format ID
	};
};
