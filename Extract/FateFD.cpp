#include "stdafx.h"
#include "../ExtractBase.h"
#include "FateFD.h"

// Function to get file information from Fate/hollow ataraxia's video.xp3 file.
BOOL CFateFD::Mount(CArcFile* pclArc)
{
	return FALSE; // TODO: Unfinished?

	if ((pclArc->GetArcExten() != _T(".xp3")) || (memcmp(pclArc->GetHed(), "XP3", 3) != 0) || (pclArc->GetHed()[0x0C] != 0x93))
		return FALSE;

	SFileInfo file[2];
	file[0].name = _T("FateFDOP1.mpg");
	file[0].start = 0x3A2;
	file[0].end = 0x551FBA6;
	file[1].name = _T("FateFDOP2.mpg");
	file[1].start = 0x551FBA6;
	file[1].end = 0x90693AA;

	// Add to listview
	for (unsigned int i = 0; i < 2; i++)
	{
		SFileInfo infFile;
		infFile.name = file[i].name;
		infFile.start = file[i].start;
		infFile.end = file[i].end;
		infFile.sizeOrg = infFile.end - infFile.start;
		infFile.sizeCmp = infFile.sizeOrg;
		pclArc->AddFileInfo(infFile);
	}

	return TRUE;
}

BOOL CFateFD::Decode(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->name.substr(0, 8) != _T("FateFDOP"))
		return FALSE;

	// Ensure buffer exists
	DWORD BufSize = pclArc->GetBufSize();
	YCMemory<BYTE> buf(BufSize);

	// Generate output files
	pclArc->OpenFile();

	DecodeInfo decode[6];
	// Preparing to decode the first file
	if (pInfFile->start == 0x3A2)
	{
		decode[0].key = 0xCE;
		decode[0].size = 0xB6B - pInfFile->start;
		decode[1].key = 0x94;
		decode[1].size = 0x7036 - 0xB6B;
		decode[2].key = 0xB3;
		decode[2].size = 0x7037 - 0x7036;
		decode[3].key = 0x94;
		decode[3].size = 0xAF27 - 0x7037;
		decode[4].key = 0xF8;
		decode[4].size = 0xAF28 - 0xAF27;
		decode[5].key = 0x94;
		decode[5].size = 0x551FBA6 - 0xAF28;
	}
	// Preparing to decode the second file.
	else
	{
		decode[0].key = 0x5A;
		decode[0].size = 0x552036D - pInfFile->start;
		decode[1].key = 0x21;
		decode[1].size = 0x552254A - 0x552036D;
		decode[2].key = 0xFE;
		decode[2].size = 0x552254B - 0x552254A;
		decode[3].key = 0x21;
		decode[3].size = 0x552CDF6 - 0x552254B;
		decode[4].key = 0xDB;
		decode[4].size = 0x552CDF7 - 0x552CDF6;
		decode[5].key = 0x21;
		decode[5].size = 0x90693AA - 0x552CDF7;
	}

	DWORD BufSize_base = BufSize;
	for (unsigned int i = 0; i < 6; i++)
	{
		BufSize = BufSize_base;
		for (DWORD DecodeSize = 0; DecodeSize != decode[i].size; DecodeSize += BufSize)
		{
			// Adjust buffer size
			if (DecodeSize + BufSize > decode[i].size)
				BufSize = decode[i].size - DecodeSize;

			// Reading
			pclArc->Read(&buf[0], BufSize);
			
			// Decryption
			for (unsigned int j = 0; j < BufSize; j++)
				buf[j] ^= decode[i].key;
				
			// Write decoded files
			pclArc->WriteFile(&buf[0], BufSize);
		}
	}

	return TRUE;
}