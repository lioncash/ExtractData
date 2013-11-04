#include "stdafx.h"
#include "../SearchBase.h"
#include "../ExtractBase.h"
#include "../Sound/Ogg.h"
#include "OggSearch.h"

COggSearch::COggSearch()
{
	//InitHed("\x4F\x67\x67\x53\x00\x02", 6);
}

void COggSearch::OnInit(SOption* pOption)
{
	if (pOption->bHighSearchOgg == TRUE)
		InitHed("\x4F\x67\x67\x53\x00\x02", 6);
	else
		InitHed("\x4F\x67\x67\x53", 4);
}

void COggSearch::Mount(CArcFile* pclArc)
{
	COgg ogg;
	ogg.Init(pclArc);

	SFileInfo infFile;

	// Get start address
	infFile.start = pclArc->GetArcPointer();

	// Serial number when reading the header from memory
	DWORD SerialNo;
	pclArc->Seek(14, FILE_CURRENT);
	pclArc->Read(&SerialNo, 4);

	pclArc->Seek(infFile.start, FILE_BEGIN);
	DWORD ReadCount = 0;
	infFile.sizeOrg = 0;

	while (1)
	{
		ReadCount++;

		// Read the OGG header
		DWORD PageSize = ogg.ReadHed();
		VH vheader = ogg.GetHed();

		//If the serial number is different than what we have
		if (vheader.serialno != SerialNo)
		{
			ogg.BackHed();
			break;
		}

		// If it is no longer OggS
		if (memcmp(vheader.pattern, GetHed(), 4) != 0)
		{
			ogg.BackHed();
			break;
		}

		// Exit after the beginning of the OGG until we hit the next header
		if ((ReadCount > 1) && (memcmp(vheader.pattern, GetHed(), 4) == 0) && (vheader.type == 2))
		{
			ogg.BackHed();
			break;
		}

		// Exit after we hit the end of the file
		if (PageSize == 0)
			break;

		// Add to the file size
		infFile.sizeOrg += PageSize;

		// Advance to the next OggS
		ogg.NextPage();

		pclArc->GetProg()->UpdatePercent(PageSize);
	}

	if (infFile.sizeOrg == 0)
		return;

	infFile.sizeCmp = infFile.sizeOrg;

	// Get exit address
	infFile.end = infFile.start + infFile.sizeOrg;

	pclArc->AddFileInfo(infFile, GetCtFile(), _T(".ogg"));
}
