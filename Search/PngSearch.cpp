#include "stdafx.h"
#include "PngSearch.h"
#include "Utils/BitUtils.h"

CPngSearch::CPngSearch()
{
	InitHed("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8);
	InitFot("\x49\x45\x4E\x44\xAE\x42\x60\x82", 8);
}

void CPngSearch::Mount(CArcFile* archive)
{
	SFileInfo file_info;

	// Get start address
	file_info.start = archive->GetArcPointer();
	archive->SeekCur(GetHedSize());

	// Search the file end
	u8 chunk_name[4];

	do
	{
		// Get chunk length
		u32 length;
		if (archive->Read(&length, 4) == 0)
			return;

		length = BitUtils::Swap32(length);

		// Get chunk name
		if (archive->Read(chunk_name, 4) == 0)
			return;

		// Advance the file pointer to chunk length + CRC segments
		// if(length + 4 > archive->GetArcSize())
		// {
		//	return;
		// }

		archive->SeekCur(length + 4);
	} while (memcmp(chunk_name, "IEND", 4) != 0); // Keep looping until IEND is reached

	// Get exit address
	file_info.end = archive->GetArcPointer();

	// Get file size
	file_info.sizeOrg = file_info.end - file_info.start;
	file_info.sizeCmp = file_info.sizeOrg;

	// Update progress bar
	archive->GetProg()->UpdatePercent(file_info.sizeOrg);

	archive->AddFileInfo(file_info, GetCtFile(), _T(".png"));
}
