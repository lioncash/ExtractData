#include "stdafx.h"
#include "../SearchBase.h"
#include "JpgSearch.h"

CJpgSearch::CJpgSearch()
{
	InitHed("\xFF\xD8\xFF\xE0**JFIF", 10);
	InitFot("\xFF\xD9", 2);
}

void CJpgSearch::Mount(CArcFile* pclArc)
{
	SFileInfo infFile;

	// Get start address
	infFile.start = pclArc->GetArcPointer();

	// Skip FFD8
	pclArc->Seek(2, FILE_CURRENT);

	// JPEG Image Search
	while (true)
	{
		// Get marker
		BYTE marker[2];
		if (pclArc->Read(marker, 2) == 0)
			return;

		// Exit the loop when we reach the JPEG image data
		if (memcmp(marker, "\xFF\xDA", 2) == 0)
			break;

		// Get the size of the data area
		WORD length;
		if (pclArc->Read(&length, 2) == 0)
			return;
		pclArc->ConvEndian(&length);

		// Advance to the next data area
		pclArc->Seek(length - 2, FILE_CURRENT);
	}

	// Advance the progress bar
	pclArc->GetProg()->UpdatePercent(pclArc->GetArcPointer() - infFile.start);

	// Get footer
	if (SearchFot(pclArc) == FALSE)
		return;

	// Get exit address
	infFile.end = pclArc->GetArcPointer();

	// Get file size
	infFile.sizeOrg = infFile.end - infFile.start;
	infFile.sizeCmp = infFile.sizeOrg;

	pclArc->AddFileInfo(infFile, GetCtFile(), _T(".jpg"));
}
