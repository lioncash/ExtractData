#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Sound/Wav.h"
#include "TrH.h"

bool CTrH::Mount(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".px")) || (memcmp(pclArc->GetHed(), "fPX ", 4) != 0))
		return false;

	return pclArc->Mount();
}

// Function to convert to WAV
bool CTrH::Decode(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if ((file_info->format != _T("PX")) || (memcmp(pclArc->GetHed(), "fPX ", 4) != 0))
		return false;

	// Read px header
	PXHed pxHed;
	pclArc->Read(&pxHed, sizeof(PXHed));

	// Output
	CWav wav;
	wav.Init(pclArc, file_info->sizeOrg - 44, pxHed.freq, pxHed.channels, pxHed.bits);
	wav.Write();

	return true;
}
