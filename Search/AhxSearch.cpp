#include "stdafx.h"
#include "AhxSearch.h"
#include "Utils/BitUtils.h"

CAhxSearch::CAhxSearch()
{
	InitHed("\x80\x00\x00\x20\x11\x00\x00\x01\x00\x00", 10);
	InitFot("AHXE(c)CRI", 10);
}

void CAhxSearch::Mount(CArcFile* pclArc)
{
	SFileInfo infFile;

	// Get start address
	infFile.start = pclArc->GetArcPointer();

	// Get file ssize
	pclArc->Seek(GetHedSize() + 2, FILE_CURRENT);
	pclArc->Read(&infFile.sizeOrg, 4);
	infFile.sizeOrg = BitUtils::Swap32(infFile.sizeOrg) << 1;
	pclArc->GetProg()->UpdatePercent(4);

	// Search footer
	if (!SearchFot(pclArc))
		return;

	// Get exit address
	infFile.end = pclArc->GetArcPointer();

	// Get compressedfile size
	infFile.sizeCmp = infFile.end - infFile.start;

	pclArc->AddFileInfo(infFile, GetCtFile(), _T(".ahx"));
}
