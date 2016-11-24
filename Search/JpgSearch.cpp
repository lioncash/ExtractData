#include "StdAfx.h"
#include "JpgSearch.h"
#include "Utils/BitUtils.h"

CJpgSearch::CJpgSearch()
{
	InitHeader("\xFF\xD8\xFF\xE0**JFIF", 10);
	InitFooter("\xFF\xD9", 2);
}

void CJpgSearch::Mount(CArcFile* archive)
{
	SFileInfo file_info;

	// Get start address
	file_info.start = archive->GetArcPointer();

	// Skip FFD8
	archive->Seek(2, FILE_CURRENT);

	// JPEG Image Search
	while (true)
	{
		// Get marker
		std::array<u8, 2> marker;
		if (archive->Read(marker.data(), marker.size()) == 0)
			return;

		// Exit the loop when we reach the JPEG image data
		if (memcmp(marker.data(), "\xFF\xDA", marker.size()) == 0)
			break;

		// Get the size of the data area
		u16 length;
		if (archive->ReadU16(&length) == 0)
			return;
		length = BitUtils::Swap16(length);

		// Advance to the next data area
		archive->Seek(length - 2, FILE_CURRENT);
	}

	// Advance the progress bar
	archive->GetProg()->UpdatePercent(archive->GetArcPointer() - file_info.start);

	// Get footer
	if (!SearchFooter(archive))
		return;

	// Get exit address
	file_info.end = archive->GetArcPointer();

	// Get file size
	file_info.sizeOrg = file_info.end - file_info.start;
	file_info.sizeCmp = file_info.sizeOrg;

	archive->AddFileInfo(file_info, GetNumFiles(), _T(".jpg"));
}
