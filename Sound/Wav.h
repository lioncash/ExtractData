#pragma once

class CArcFile;

class CWav final
{
public:
	void Init(CArcFile* pclArc, DWORD dataSize, DWORD freq, WORD channels, WORD bits);
	void Write();
	void Write(LPBYTE buf);
	void Write(LPBYTE buf, DWORD size);

private:
	// WAV Format
	struct WAVHed
	{
		BYTE    RiffID[4]; // "RIFF"
		DWORD   FileSize;  // Filesize
		BYTE    WaveID[4]; // "WAVE"
		BYTE    fmtID[4];  // "fmt "
		DWORD   ChunkByte; // fmt Number of bytes in chunk
		WORD    FormatID;  // Format ID
		WORD    channels;  // Number of channels
		DWORD   freq;      // Sampling rate
		DWORD   bps;       // Data rate (Byte/sec)
		WORD    BlockSize; // Block size (Byte/sample * Number of channels)
		WORD    bits;      // Number of bits per sample (bit/sample)
		BYTE    DataID[4]; // "data"
		DWORD   DataSize;  // Number of bytes of waveform data
	};

	CArcFile* m_archive = nullptr;
	WAVHed m_wavHed{};
};
