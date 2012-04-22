
#include	"stdafx.h"
#include	"MidSearch.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	Constructor

CMidSearch::CMidSearch()
{
	InitHed( "MThd\0\0\0\x06", 8 );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Mount

void	CMidSearch::Mount(
	CArcFile*			pclArc							// Archive
	)
{
	SFileInfo			stFileInfo;

	// Get the head position

	stFileInfo.start = pclArc->GetArcPointer();

	// Get the number of tracks

	WORD				wTracks;

	pclArc->SeekCur( (GetHedSize() + 2) );
	pclArc->Read( &wTracks, 2 );
	pclArc->ConvEndian( &wTracks );

	pclArc->SeekCur( 2 );

	// Get end positions

	for( WORD wCnt = 0 ; wCnt < wTracks ; wCnt++ )
	{
		BYTE				abtMark[4];

		pclArc->Read( abtMark, 4 );

		if( memcmp( abtMark, "MTrk", 4 ) != 0 )
		{
			// Invalid MIDI

			return;
		}

		// Get track size

		DWORD				dwTrackSize;

		pclArc->Read( &dwTrackSize, 4 );
		pclArc->ConvEndian( &dwTrackSize );

		// Advance to next track

		pclArc->SeekCur( dwTrackSize );
	}

	stFileInfo.end = pclArc->GetArcPointer();

	// Get file size

	stFileInfo.sizeOrg = stFileInfo.end - stFileInfo.start;
	stFileInfo.sizeCmp = stFileInfo.sizeOrg;

	// Update progress bar

	pclArc->GetProg()->UpdatePercent( stFileInfo.sizeOrg );

	pclArc->AddFileInfo( stFileInfo, GetCtFile(), _T(".mid") );
}
