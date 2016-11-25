#include "StdAfx.h"
#include "MidSearch.h"

#include "ArcFile.h"
#include "Utils/BitUtils.h"

CMidSearch::CMidSearch()
{
	InitHeader("MThd\0\0\0\x06", 8);
}

void CMidSearch::Mount(CArcFile* archive)
{
	SFileInfo file_info;

	// Get the head position
	file_info.start = archive->GetArcPointer();

	// Get the number of tracks
	u16 tracks;
	archive->SeekCur(GetHeaderSize() + 2);
	archive->ReadU16(&tracks);
	tracks = BitUtils::Swap16(tracks);
	archive->SeekCur(2);

	// Get end positions
	for (unsigned short cnt = 0; cnt < tracks; cnt++)
	{
		std::array<u8, 4> marker;
		archive->Read(marker.data(), marker.size());

		// Invalid MIDI
		if (memcmp(marker.data(), "MTrk", marker.size()) != 0)
		{
			return;
		}

		// Get track size
		u32 track_size;
		archive->ReadU32(&track_size);
		track_size = BitUtils::Swap32(track_size);

		// Advance to next track
		archive->SeekCur(track_size);
	}

	file_info.end = archive->GetArcPointer();

	// Get file size
	file_info.sizeOrg = file_info.end - file_info.start;
	file_info.sizeCmp = file_info.sizeOrg;

	// Update progress bar
	archive->GetProg()->UpdatePercent(file_info.sizeOrg);

	archive->AddFileInfo(file_info, GetNumFiles(), _T(".mid"));
}
