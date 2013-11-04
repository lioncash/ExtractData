#include "stdafx.h"
#include "../SearchBase.h"
#include "MpgSearch.h"

CMpgSearch::CMpgSearch()
{
	InitHed("\x00\x00\x01\xBA\x21\x00\x01\x00", 8);
	InitFot("\xFF\xFF\xFF\xFF\x00\x00\x01\xB9", 8);
}

void CMpgSearch::Mount(CArcFile* pclArc)
{
	// Corresponds to CVM
	LPCTSTR pFileExt = (lstrcmpi(pclArc->GetArcExten(), _T(".cvm")) == 0) ? _T(".sfd") : _T(".mpg");

	SFileInfo infFile;

	// Get start address
	infFile.start = pclArc->GetArcPointer();
	pclArc->Seek(GetHedSize(), FILE_CURRENT);
	pclArc->GetProg()->UpdatePercent(GetHedSize());

	// Get footer
	if (SearchFot(pclArc) == FALSE)
		return;

	// Get exit address
	infFile.end = pclArc->GetArcPointer();

	// Get file size
	infFile.sizeOrg = infFile.end - infFile.start;
	infFile.sizeCmp = infFile.sizeOrg;

	pclArc->AddFileInfo(infFile, GetCtFile(), pFileExt);
}
