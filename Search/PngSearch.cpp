#include "StdAfx.h"
#include "Search/PngSearch.h"

#include "ArcFile.h"
#include "Utils/BitUtils.h"

CPngSearch::CPngSearch()
{
	InitHeader("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8);
	InitFooter("\x49\x45\x4E\x44\xAE\x42\x60\x82", 8);
}

void CPngSearch::Mount(CArcFile* archive)
{
	SFileInfo file_info;

	// Get start address
	file_info.start = archive->GetArcPointer();
	archive->SeekCur(GetHeaderSize());

	// Search the file end
	std::array<u8, 4> chunk_name;

	do
	{
		// Get chunk length
		u32 length;
		if (archive->ReadU32(&length) == 0)
			return;

		length = BitUtils::Swap32(length);

		// Get chunk name
		if (archive->Read(chunk_name.data(), chunk_name.size()) == 0)
			return;

		// Advance the file pointer to chunk length + CRC segments
		// if(length + 4 > archive->GetArcSize())
		// {
		//	return;
		// }

		archive->SeekCur(length + 4);
	} while (std::memcmp(chunk_name.data(), "IEND", chunk_name.size()) != 0); // Keep looping until IEND is reached

	// Get exit address
	file_info.end = archive->GetArcPointer();

	// Get file size
	file_info.size_org = file_info.end - file_info.start;
	file_info.size_cmp = file_info.size_org;

	// Update progress bar
	archive->GetProg()->UpdatePercent(file_info.size_org);

	archive->AddFileInfo(file_info, GetNumFiles(), _T(".png"));
}
