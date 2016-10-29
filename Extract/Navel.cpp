#include "stdafx.h"
#include "../Sound/Wav.h"
#include "Navel.h"

// Function that gets file information from Navel's .pac files
bool CNavel::Mount(CArcFile* pclArc)
{
	if (MountPac(pclArc))
		return true;
	if (MountWpd(pclArc))
		return true;

	return false;
}

bool CNavel::MountPac(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".pac")) || (memcmp(pclArc->GetHed(), "CAPF", 4) != 0))
		return false;

	// Get index size
	DWORD index_size;
	pclArc->Seek(8, FILE_BEGIN);
	pclArc->Read(&index_size, 4);
	index_size -= 32;

	// Get file count
	DWORD ctFile;
	pclArc->Read(&ctFile, 4);

	// Get index
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Seek(16, FILE_CURRENT);
	pclArc->Read(pIndex, index_size);

	for (DWORD i = 0; i < ctFile; i++)
	{
		// Get file name
		TCHAR szFileName[32];
		memcpy(szFileName, &pIndex[8], 32);

		// Add to list view
		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.sizeOrg = *(LPDWORD)&pIndex[4];
		infFile.sizeCmp = infFile.sizeOrg;
		infFile.start = *(LPDWORD)&pIndex[0];
		infFile.end = infFile.start + infFile.sizeOrg;
		pclArc->AddFileInfo(infFile);

		pIndex += 40;
	}

	return true;
}

bool CNavel::MountWpd(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".WPD")) || (memcmp(pclArc->GetHed(), " DPW", 4) != 0))
		return false;

	return pclArc->Mount();
}

// Function to convert to WAV files
bool CNavel::Decode(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if (file_info->format != _T("WPD"))
		return false;

	// Read WPD Format
	FormatWPD fWPD;
	pclArc->Read(&fWPD, sizeof(FormatWPD));

	// Output
	CWav wav;
	wav.Init(pclArc, file_info->sizeOrg - 44, fWPD.freq, fWPD.channels, fWPD.bits);
	wav.Write();

	return true;
}
