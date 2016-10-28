#include "stdafx.h"
#include "BmpSearch.h"

CBmpSearch::CBmpSearch()
{
	InitHed("BM****\0\0\0\0\x36*\0\0\x28\0\0\0", 18);
}

void CBmpSearch::Mount(CArcFile* pclArc)
{
	SFileInfo infFile;

	// Get start address
	infFile.start = pclArc->GetArcPointer();

	// Get file size
	pclArc->Seek(2, FILE_CURRENT);
	pclArc->Read(&infFile.sizeOrg, 4);

	// Corresponds to the case filesize
	if (infFile.sizeOrg == 0)
	{
		// Get the full size
		pclArc->Seek(16, FILE_CURRENT);
		pclArc->Read(&infFile.sizeOrg, 4);

		// If it is not written, also search the end of the BMP (avoids an infinite loop)
		if (infFile.sizeOrg == 0)
			return;

		// +54(BMP header size)
		infFile.sizeOrg += 54;
	}

	infFile.sizeCmp = infFile.sizeOrg;

	// Get exit address
	infFile.end = infFile.start + infFile.sizeOrg;

	// Find the end of the BMP file
	pclArc->Seek(infFile.end, FILE_BEGIN);
	pclArc->GetProg()->UpdatePercent(infFile.sizeOrg);

	pclArc->AddFileInfo(infFile, GetCtFile(), _T(".bmp"));
}
