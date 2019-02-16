#include "StdAfx.h"
#include "Search/MpgSearch.h"

#include "ArcFile.h"

CMpgSearch::CMpgSearch()
{
	InitHeader("\x00\x00\x01\xBA\x21\x00\x01\x00", 8);
	InitFooter("\xFF\xFF\xFF\xFF\x00\x00\x01\xB9", 8);
}

void CMpgSearch::Mount(CArcFile* archive)
{
	// Corresponds to CVM
	LPCTSTR file_extension = (lstrcmpi(archive->GetArcExten(), _T(".cvm")) == 0) ? _T(".sfd") : _T(".mpg");

	SFileInfo file_info;

	// Get start address
	file_info.start = archive->GetArcPointer();
	archive->Seek(GetHeaderSize(), FILE_CURRENT);
	archive->GetProg()->UpdatePercent(GetHeaderSize());

	// Get footer
	if (!SearchFooter(archive))
		return;

	// Get exit address
	file_info.end = archive->GetArcPointer();

	// Get file size
	file_info.size_org = file_info.end - file_info.start;
	file_info.size_cmp = file_info.size_org;

	archive->AddFileInfo(file_info, GetNumFiles(), file_extension);
}
