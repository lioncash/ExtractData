#include "stdafx.h"
#include "WmvSearch.h"

CWmvSearch::CWmvSearch()
{
	InitHed("\x30\x26\xB2\x75\x8E\x66\xCF\x11\xA6\xD9\x00\xAA\x00\x62\xCE\x6C", 16);
	InitFot("\xA1\xDC\xAB\x8C\x47\xA9\xCF\x11\x8E\xE4\x00\xC0\x0C\x20\x53\x65\x68", 17);
}

void CWmvSearch::Mount(CArcFile* archive)
{
	SFileInfo file_info;

	// Get start address
	file_info.start = archive->GetArcPointer();
	archive->Seek(GetHedSize(), FILE_CURRENT);

	// Search the auxiliary header
	if (!SearchFot(archive))
		return;

	// Amount of progress advanced by the search bar
	const u64 search_offset = archive->GetArcPointer() - file_info.start - GetHedSize();

	// Get file size
	archive->Seek(23, FILE_CURRENT);
	archive->Read(&file_info.sizeOrg, 4);
	file_info.sizeCmp = file_info.sizeOrg;

	// Get exit address
	file_info.end = file_info.start + file_info.sizeOrg;

	// Go to the end of the WMV file
	archive->Seek(file_info.end, FILE_BEGIN);
	archive->GetProg()->UpdatePercent(file_info.sizeOrg - search_offset);

	archive->AddFileInfo(file_info, GetCtFile(), _T(".wmv"));
}
