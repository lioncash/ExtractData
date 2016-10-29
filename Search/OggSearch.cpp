#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Sound/Ogg.h"
#include "OggSearch.h"

COggSearch::COggSearch()
{
	//InitHed("\x4F\x67\x67\x53\x00\x02", 6);
}

void COggSearch::OnInit(SOption* option)
{
	if (option->bHighSearchOgg)
		InitHed("\x4F\x67\x67\x53\x00\x02", 6);
	else
		InitHed("\x4F\x67\x67\x53", 4);
}

void COggSearch::Mount(CArcFile* archive)
{
	COgg ogg;
	ogg.Init(archive);

	SFileInfo file_info;

	// Get start address
	file_info.start = archive->GetArcPointer();

	// Serial number when reading the header from memory
	u32 serial_no;
	archive->Seek(14, FILE_CURRENT);
	archive->Read(&serial_no, 4);

	archive->Seek(file_info.start, FILE_BEGIN);
	u32 read_count = 0;
	file_info.sizeOrg = 0;

	while (true)
	{
		read_count++;

		// Read the OGG header
		const u32 page_size = ogg.ReadHed();
		VH header = ogg.GetHed();

		//If the serial number is different than what we have
		if (header.serialno != serial_no)
		{
			ogg.BackHed();
			break;
		}

		// If it is no longer OggS
		if (memcmp(header.pattern, GetHed(), 4) != 0)
		{
			ogg.BackHed();
			break;
		}

		// Exit after the beginning of the OGG until we hit the next header
		if (read_count > 1 && memcmp(header.pattern, GetHed(), 4) == 0 && header.type == 2)
		{
			ogg.BackHed();
			break;
		}

		// Exit after we hit the end of the file
		if (page_size == 0)
			break;

		// Add to the file size
		file_info.sizeOrg += page_size;

		// Advance to the next OggS
		ogg.NextPage();

		archive->GetProg()->UpdatePercent(page_size);
	}

	if (file_info.sizeOrg == 0)
		return;

	file_info.sizeCmp = file_info.sizeOrg;

	// Get exit address
	file_info.end = file_info.start + file_info.sizeOrg;

	archive->AddFileInfo(file_info, GetCtFile(), _T(".ogg"));
}
