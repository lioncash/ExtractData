#include "stdafx.h"
#include "../SearchBase.h"
#include "WmvSearch.h"

CWmvSearch::CWmvSearch()
{
	InitHed("\x30\x26\xB2\x75\x8E\x66\xCF\x11\xA6\xD9\x00\xAA\x00\x62\xCE\x6C", 16);
	InitFot("\xA1\xDC\xAB\x8C\x47\xA9\xCF\x11\x8E\xE4\x00\xC0\x0C\x20\x53\x65\x68", 17);
}

void CWmvSearch::Mount(CArcFile* pclArc)
{
	SFileInfo infFile;

	// Get start address
	infFile.start = pclArc->GetArcPointer();
	pclArc->Seek(GetHedSize(), FILE_CURRENT);

	// Search the auxiliary header
	if (SearchFot(pclArc) == FALSE)
		return;

	// Amount of progress advanced by the search bar
	QWORD search_offset = pclArc->GetArcPointer() - infFile.start - GetHedSize();

	// Get file size
	pclArc->Seek(23, FILE_CURRENT);
	pclArc->Read(&infFile.sizeOrg, 4);
	infFile.sizeCmp = infFile.sizeOrg;

	// Get exit address
	infFile.end = infFile.start + infFile.sizeOrg;

	// Go to the end of the WMV file
	pclArc->Seek(infFile.end, FILE_BEGIN);
	pclArc->GetProg()->UpdatePercent(infFile.sizeOrg - search_offset);

	pclArc->AddFileInfo(infFile, GetCtFile(), _T(".wmv"));
}
