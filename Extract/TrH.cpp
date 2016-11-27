#include "StdAfx.h"
#include "Extract/TrH.h"

#include "ArcFile.h"
#include "Sound/Wav.h"

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
	PXHed header;
	archive->Read(&header, sizeof(PXHed));

	// Output
	CWav wav;
	wav.Init(archive, file_info->sizeOrg - 44, header.freq, header.channels, header.bits);
	wav.Write();

	return true;
}
