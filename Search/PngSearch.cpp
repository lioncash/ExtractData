#include "stdafx.h"
#include "../SearchBase.h"
#include "PngSearch.h"
#include "Utils/BitUtils.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

CPngSearch::CPngSearch()
{
	InitHed("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8);
	InitFot("\x49\x45\x4E\x44\xAE\x42\x60\x82", 8);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Mount

void CPngSearch::Mount(CArcFile* pclArc)
{
	SFileInfo stFileInfo;

	// Get start address
	stFileInfo.start = pclArc->GetArcPointer();
	pclArc->SeekCur(GetHedSize());

	// Search the file end
	BYTE abtChunkName[4];

	do
	{
		// Get chunk length
		DWORD dwLength;
		if (pclArc->Read(&dwLength, 4) == 0)
			return;

		dwLength = BitUtils::Swap32(dwLength);

		// Get chunk name
		if (pclArc->Read(abtChunkName, 4) == 0)
			return;

		// Advance the file pointer to chunk length + CRC segments
		// if( (dwLength + 4) > pclArc->GetArcSize() )
		// {
		//	return;
		// }

		pclArc->SeekCur(dwLength + 4);
	} while (memcmp(abtChunkName, "IEND", 4) != 0); // Keep looping until IEND is reached

	// Get exit address
	stFileInfo.end = pclArc->GetArcPointer();

	// Get file size
	stFileInfo.sizeOrg = stFileInfo.end - stFileInfo.start;
	stFileInfo.sizeCmp = stFileInfo.sizeOrg;

	// Update progress bar
	pclArc->GetProg()->UpdatePercent(stFileInfo.sizeOrg);

	pclArc->AddFileInfo(stFileInfo, GetCtFile(), _T(".png"));
}
