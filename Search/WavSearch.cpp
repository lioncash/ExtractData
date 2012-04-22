#include "stdafx.h"
#include "../SearchBase.h"
#include "WavSearch.h"

CWavSearch::CWavSearch()
{
	InitHed("RIFF****WAVE", 12);
}

void CWavSearch::Mount(CArcFile* pclArc)
{
	SFileInfo infFile;

	// Get start address
	infFile.start = pclArc->GetArcPointer();

	// Get file size
	pclArc->Seek(4, FILE_CURRENT);
	pclArc->Read(&infFile.sizeOrg, 4);
	infFile.sizeOrg += 8;
	infFile.sizeCmp = infFile.sizeOrg;

	// Get exit address
	infFile.end = infFile.start + infFile.sizeOrg;

	// Go to the end of the WAV file
	pclArc->Seek(infFile.end, FILE_BEGIN);
	pclArc->GetProg()->UpdatePercent(infFile.sizeOrg);

	pclArc->AddFileInfo(infFile, GetCtFile(), _T(".wav"));
}