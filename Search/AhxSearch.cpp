#include "stdafx.h"
#include "AhxSearch.h"
#include "Utils/BitUtils.h"

CAhxSearch::CAhxSearch()
{
	InitHed("\x80\x00\x00\x20\x11\x00\x00\x01\x00\x00", 10);
	InitFot("AHXE(c)CRI", 10);
}

void CAhxSearch::Mount(CArcFile* archive)
{
	SFileInfo file_info;

	// Get start address
	file_info.start = archive->GetArcPointer();

	// Get file ssize
	archive->Seek(GetHedSize() + 2, FILE_CURRENT);
	archive->Read(&file_info.sizeOrg, 4);
	file_info.sizeOrg = BitUtils::Swap32(file_info.sizeOrg) << 1;
	archive->GetProg()->UpdatePercent(4);

	// Search footer
	if (!SearchFot(archive))
		return;

	// Get exit address
	file_info.end = archive->GetArcPointer();

	// Get compressedfile size
	file_info.sizeCmp = file_info.end - file_info.start;

	archive->AddFileInfo(file_info, GetCtFile(), _T(".ahx"));
}
