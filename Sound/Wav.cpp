#include "stdafx.h"
#include "../ArcFile.h"
#include "Wav.h"

void CWav::Init(CArcFile* archive, u32 data_size, u32 freq, u16 channels, u16 bits)
{
	m_archive = archive;

	InitHeader(data_size, freq, channels, bits);

	m_archive->OpenFile(_T(".wav"));
	m_archive->WriteFile(&m_header, sizeof(Header));
}

void CWav::InitHeader(u32 data_size, u32 freq, u16 channels, u16 bits)
{
	memcpy(m_header.riff_id, "RIFF", 4);
	m_header.file_size = data_size + 36;
	memcpy(m_header.wave_id, "WAVE", 4);
	memcpy(m_header.fmt_id, "fmt ", 4);
	m_header.chunk_byte = 0x10; // Linear PCM always converted to 16
	m_header.format_id = 0x01; // Linear PCM always converted to 1
	m_header.channels = channels;
	m_header.freq = freq;
	m_header.bps = freq * (bits >> 3) * channels;
	m_header.block_size = (bits >> 3) * channels;
	m_header.bits = bits;
	memcpy(m_header.data_id, "data", 4);
	m_header.data_size = data_size;
}

void CWav::Write()
{
	m_archive->ReadWrite(m_header.data_size);
}

void CWav::Write(const u8* buf)
{
	m_archive->WriteFile(buf, m_header.data_size);
}

void CWav::Write(const u8* buf, size_t size)
{
	m_archive->WriteFile(buf, size);
}
