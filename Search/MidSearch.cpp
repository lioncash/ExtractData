#include "stdafx.h"
#include "MidSearch.h"
#include "Utils/BitUtils.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

CMidSearch::CMidSearch()
{
	InitHed("MThd\0\0\0\x06", 8);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Mount

void CMidSearch::Mount(CArcFile* archive)
{
	SFileInfo file_info;

	// Get the head position
	file_info.start = archive->GetArcPointer();

	// Get the number of tracks
	u16 tracks;
	archive->SeekCur(GetHedSize() + 2);
	archive->Read(&tracks, sizeof(u16));
	tracks = BitUtils::Swap16(tracks);
	archive->SeekCur(2);

	// Get end positions
	for (unsigned short cnt = 0; cnt < tracks; cnt++)
	{
		u8 marker[4];
		archive->Read(marker, 4);

		// Invalid MIDI
		if (memcmp(marker, "MTrk", 4) != 0)
		{
			return;
		}

		// Get track size
		u32 track_size;
		archive->Read(&track_size, sizeof(u32));
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

	archive->AddFileInfo(file_info, GetCtFile(), _T(".mid"));
}
