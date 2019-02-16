#include "StdAfx.h"
#include "Search/BmpSearch.h"

#include "ArcFile.h"
#include "Common.h"
#include "UI/ProgressBar.h"

CBmpSearch::CBmpSearch()
{
	InitHeader("BM****\0\0\0\0\x36*\0\0\x28\0\0\0", 18);
}

void CBmpSearch::Mount(CArcFile* archive)
{
	SFileInfo file_info;

	// Get start address
	file_info.start = archive->GetArcPointer();

	// Get file size
	archive->Seek(2, FILE_CURRENT);
	archive->Read(&file_info.size_org, 4);

	// Corresponds to the case filesize
	if (file_info.size_org == 0)
	{
		// Get the full size
		archive->Seek(16, FILE_CURRENT);
		archive->Read(&file_info.size_org, 4);

		// If it is not written, also search the end of the BMP (avoids an infinite loop)
		if (file_info.size_org == 0)
			return;

		// +54(BMP header size)
		file_info.size_org += 54;
	}

	file_info.size_cmp = file_info.size_org;

	// Get exit address
	file_info.end = file_info.start + file_info.size_org;

	// Find the end of the BMP file
	archive->Seek(file_info.end, FILE_BEGIN);
	archive->GetProg()->UpdatePercent(file_info.size_org);

	archive->AddFileInfo(file_info, GetNumFiles(), _T(".bmp"));
}
