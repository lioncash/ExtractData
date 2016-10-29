#include "stdafx.h"
#include "../ArcFile.h"
#include "Wav.h"

void CWav::Init(CArcFile* archive, u32 dataSize, u32 freq, u16 channels, u16 bits)
{
	m_archive = archive;
	WAVHed* wavHed = &m_wavHed;

	memcpy(wavHed->riff_id, "RIFF", 4);
	wavHed->file_size = dataSize + 36;
	memcpy(wavHed->wave_id, "WAVE", 4);
	memcpy(wavHed->fmt_id, "fmt ", 4);
	wavHed->chunk_byte = 0x10; // Linear PCM always converted to 16
	wavHed->format_id = 0x01; // Linear PCM always converted to 1
	wavHed->channels = channels;
	wavHed->freq = freq;
	wavHed->bps = freq * (bits >> 3) * channels;
	wavHed->block_size = (bits >> 3) * channels;
	wavHed->bits = bits;
	memcpy(wavHed->data_id, "data", 4);
	wavHed->data_size = dataSize;

	m_archive->OpenFile(_T(".wav"));
	m_archive->WriteFile(wavHed, 44);
}

void CWav::Write()
{
	m_archive->ReadWrite(m_wavHed.data_size);
}

void CWav::Write(u8* buf)
{
	m_archive->WriteFile(buf, m_wavHed.data_size);
}

void CWav::Write(u8* buf, size_t size)
{
	m_archive->WriteFile(buf, size);
}
