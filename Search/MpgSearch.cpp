#include "stdafx.h"
#include "MpgSearch.h"

CMpgSearch::CMpgSearch()
{
	InitHed("\x00\x00\x01\xBA\x21\x00\x01\x00", 8);
	InitFot("\xFF\xFF\xFF\xFF\x00\x00\x01\xB9", 8);
}

void CMpgSearch::Mount(CArcFile* archive)
{
	// Corresponds to CVM
	LPCTSTR file_extension = (lstrcmpi(archive->GetArcExten(), _T(".cvm")) == 0) ? _T(".sfd") : _T(".mpg");

	SFileInfo file_info;

	// Get start address
	file_info.start = archive->GetArcPointer();
	archive->Seek(GetHedSize(), FILE_CURRENT);
	archive->GetProg()->UpdatePercent(GetHedSize());

	// Get footer
	if (!SearchFot(archive))
		return;

	// Get exit address
	file_info.end = archive->GetArcPointer();

	// Get file size
	file_info.sizeOrg = file_info.end - file_info.start;
	file_info.sizeCmp = file_info.sizeOrg;

	archive->AddFileInfo(file_info, GetCtFile(), file_extension);
}
