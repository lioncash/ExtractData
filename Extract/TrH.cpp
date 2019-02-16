#include "StdAfx.h"
#include "Extract/TrH.h"

#include "ArcFile.h"
#include "Sound/Wav.h"

namespace
{
// PX Format
struct PXHeader
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
	u32 chunk_byte;     // Number of bytes per chunk
	u16 bits;           // Number of bits per sample(bit/sample)
	u16 format_id;      // Format ID
};
} // Anonymous namespace

bool CTrH::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".px") || memcmp(archive->GetHeader(), "fPX ", 4) != 0)
		return false;

	return archive->Mount();
}

// Function to convert to WAV
bool CTrH::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("PX") || memcmp(archive->GetHeader(), "fPX ", 4) != 0)
		return false;

	// Read px header
	PXHeader header;
	archive->Read(&header, sizeof(PXHeader));

	// Output
	CWav wav;
	wav.Init(archive, file_info->size_org - 44, header.freq, header.channels, header.bits);
	wav.Write();

	return true;
}
