#include "StdAfx.h"
#include "WavSearch.h"

CWavSearch::CWavSearch()
{
	InitHed("RIFF****WAVE", 12);
}

void CWavSearch::Mount(CArcFile* archive)
{
	SFileInfo file_info;

	// Get start address
	file_info.start = archive->GetArcPointer();

	// Get file size
	archive->Seek(4, FILE_CURRENT);
	archive->Read(&file_info.sizeOrg, 4);
	file_info.sizeOrg += 8;
	file_info.sizeCmp = file_info.sizeOrg;

	// Get exit address
	file_info.end = file_info.start + file_info.sizeOrg;

	// Go to the end of the WAV file
	archive->Seek(file_info.end, FILE_BEGIN);
	archive->GetProg()->UpdatePercent(file_info.sizeOrg);

	archive->AddFileInfo(file_info, GetCtFile(), _T(".wav"));
}
