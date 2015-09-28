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

void CMidSearch::Mount(CArcFile* pclArc)
{
	SFileInfo stFileInfo;

	// Get the head position
	stFileInfo.start = pclArc->GetArcPointer();

	// Get the number of tracks
	unsigned short wTracks;
	pclArc->SeekCur((GetHedSize() + 2));
	pclArc->Read(&wTracks, sizeof(unsigned short));
	wTracks = BitUtils::Swap16(wTracks);
	pclArc->SeekCur(2);

	// Get end positions
	for (unsigned short wCnt = 0; wCnt < wTracks; wCnt++)
	{
		BYTE abtMark[4];
		pclArc->Read(abtMark, 4);

		// Invalid MIDI
		if (memcmp(abtMark, "MTrk", 4) != 0)
		{
			return;
		}

		// Get track size
		unsigned int dwTrackSize;
		pclArc->Read(&dwTrackSize, sizeof(unsigned int));
		dwTrackSize = BitUtils::Swap32(dwTrackSize);

		// Advance to next track
		pclArc->SeekCur(dwTrackSize);
	}

	stFileInfo.end = pclArc->GetArcPointer();

	// Get file size
	stFileInfo.sizeOrg = stFileInfo.end - stFileInfo.start;
	stFileInfo.sizeCmp = stFileInfo.sizeOrg;

	// Update progress bar
	pclArc->GetProg()->UpdatePercent(stFileInfo.sizeOrg);

	pclArc->AddFileInfo(stFileInfo, GetCtFile(), _T(".mid"));
}
