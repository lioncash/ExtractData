#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Wav.h"
#include "TrH.h"

BOOL CTrH::Mount(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".px")) || (memcmp(pclArc->GetHed(), "fPX ", 4) != 0))
		return FALSE;

	return pclArc->Mount();
}

// Function to convert to WAV
BOOL CTrH::Decode(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if ((pInfFile->format != _T("PX")) || (memcmp(pclArc->GetHed(), "fPX ", 4) != 0))
		return FALSE;

	// Read px header
	PXHed pxHed;
	pclArc->Read(&pxHed, sizeof(PXHed));

	// Output
	CWav wav;
	wav.Init(pclArc, pInfFile->sizeOrg - 44, pxHed.freq, pxHed.channels, pxHed.bits);
	wav.Write();

	return TRUE;
}
