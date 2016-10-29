#include "stdafx.h"
#include "../ArcFile.h"
#include "Wav.h"

void CWav::Init(CArcFile* archive, DWORD dataSize, DWORD freq, WORD channels, WORD bits)
{
	m_archive = archive;
	WAVHed* wavHed = &m_wavHed;

	memcpy(wavHed->RiffID, "RIFF", 4);
	wavHed->FileSize = dataSize + 36;
	memcpy(wavHed->WaveID, "WAVE", 4);
	memcpy(wavHed->fmtID, "fmt ", 4);
	wavHed->ChunkByte = 0x10; // Linear PCM always converted to 16
	wavHed->FormatID = 0x01; // Linear PCM always converted to 1
	wavHed->channels = channels;
	wavHed->freq = freq;
	wavHed->bps = freq * (bits >> 3) * channels;
	wavHed->BlockSize = (bits >> 3) * channels;
	wavHed->bits = bits;
	memcpy(wavHed->DataID, "data", 4);
	wavHed->DataSize = dataSize;

	m_archive->OpenFile(_T(".wav"));
	m_archive->WriteFile(wavHed, 44);
}

void CWav::Write()
{
	m_archive->ReadWrite(m_wavHed.DataSize);
}

void CWav::Write(LPBYTE buf)
{
	m_archive->WriteFile(buf, m_wavHed.DataSize);
}

void CWav::Write(LPBYTE buf, DWORD size)
{
	m_archive->WriteFile(buf, size);
}
