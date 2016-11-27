#include "StdAfx.h"
#include "Search/AhxSearch.h"

#include "ArcFile.h"
#include "Utils/BitUtils.h"

CAhxSearch::CAhxSearch()
{
	InitHeader("\x80\x00\x00\x20\x11\x00\x00\x01\x00\x00", 10);
	InitFooter("AHXE(c)CRI", 10);
}

void CAhxSearch::Mount(CArcFile* archive)
{
	SFileInfo file_info;

	// Get start address
	file_info.start = archive->GetArcPointer();

	// Get file ssize
	archive->Seek(GetHeaderSize() + 2, FILE_CURRENT);
	archive->Read(&file_info.sizeOrg, 4);
	file_info.sizeOrg = BitUtils::Swap32(file_info.sizeOrg) << 1;
	archive->GetProg()->UpdatePercent(4);

	// Search footer
	if (!SearchFooter(archive))
		return;

	// Get exit address
	file_info.end = archive->GetArcPointer();

	// Get compressedfile size
	file_info.sizeCmp = file_info.end - file_info.start;

	archive->AddFileInfo(file_info, GetNumFiles(), _T(".ahx"));
}
