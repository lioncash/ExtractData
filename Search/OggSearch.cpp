#include "StdAfx.h"
#include "Search/OggSearch.h"

#include "ArcFile.h"
#include "Sound/Ogg.h"

COggSearch::COggSearch() = default;

void COggSearch::OnInit(const SOption* option)
{
	if (option->bHighSearchOgg)
		InitHeader("\x4F\x67\x67\x53\x00\x02", 6);
	else
		InitHeader("\x4F\x67\x67\x53", 4);
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
	archive->ReadU32(&serial_no);

	archive->Seek(file_info.start, FILE_BEGIN);
	u32 read_count = 0;
	file_info.sizeOrg = 0;

	while (true)
	{
		read_count++;

		// Read the OGG header
		const u32 page_size = ogg.ReadHeader();
		OggPageHeader header = ogg.GetHeader();

		//If the serial number is different than what we have
		if (header.serial_no != serial_no)
		{
			ogg.SeekToPreviousHeader();
			break;
		}

		// If it is no longer OggS
		if (std::memcmp(header.pattern, GetHeader(), 4) != 0)
		{
			ogg.SeekToPreviousHeader();
			break;
		}

		// Exit after the beginning of the OGG until we hit the next header
		if (read_count > 1 && std::memcmp(header.pattern, GetHeader(), 4) == 0 && header.type == 2)
		{
			ogg.SeekToPreviousHeader();
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

	archive->AddFileInfo(file_info, GetNumFiles(), _T(".ogg"));
}
