#include "StdAfx.h"
#include "Search/MidSearch.h"

#include "ArcFile.h"
#include "Common.h"
#include "UI/ProgressBar.h"
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
		if (std::memcmp(marker.data(), "MTrk", marker.size()) != 0)
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
	file_info.size_org = file_info.end - file_info.start;
	file_info.size_cmp = file_info.size_org;

	// Update progress bar
	archive->GetProg()->UpdatePercent(file_info.size_org);

	archive->AddFileInfo(file_info, GetNumFiles(), _T(".mid"));
}
