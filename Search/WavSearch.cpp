#include "StdAfx.h"
#include "Search/WavSearch.h"

#include "ArcFile.h"
#include "Common.h"
#include "UI/ProgressBar.h"

CWavSearch::CWavSearch()
{
	InitHeader("RIFF****WAVE", 12);
}

void CWavSearch::Mount(CArcFile* archive)
{
	SFileInfo file_info;

	// Get start address
	file_info.start = archive->GetArcPointer();

	// Get file size
	archive->Seek(4, FILE_CURRENT);
	archive->Read(&file_info.size_org, 4);
	file_info.size_org += 8;
	file_info.size_cmp = file_info.size_org;

	// Get exit address
	file_info.end = file_info.start + file_info.size_org;

	// Go to the end of the WAV file
	archive->Seek(file_info.end, FILE_BEGIN);
	archive->GetProg()->UpdatePercent(file_info.size_org);

	archive->AddFileInfo(file_info, GetNumFiles(), _T(".wav"));
}
