#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Arc/Zlib.h"
#include "Windmill.h"

BOOL CWindmill::Mount(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".int")) || (memcmp(pclArc->GetHed(), "KIF", 3) != 0))
		return FALSE;

	// Get file count
	DWORD ctFile;
	pclArc->Seek(4, FILE_BEGIN);
	pclArc->Read(&ctFile, 4);

	// Number of files retrieved from the index size
	DWORD index_size = ctFile * 40;

	// Get index
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Read(pIndex, index_size);

	for (DWORD i = 0; i < ctFile; i++)
	{
		// Get file name
		TCHAR szFileName[32];
		memcpy(szFileName, pIndex, 32);

		// Add to listview
		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.start = *(LPDWORD)&pIndex[32];
		infFile.sizeCmp = *(LPDWORD)&pIndex[36];
		infFile.sizeOrg = infFile.sizeCmp;
		infFile.end = infFile.start + infFile.sizeCmp;
		pclArc->AddFileInfo(infFile);

		pIndex += 40;
	}

	return TRUE;
}

BOOL CWindmill::Decode(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->format != _T("HG2"))
		return FALSE;

	return FALSE; // TODO: Not completed yet

	// Read HG2 header
	BYTE header[48];
	pclArc->Read(header, sizeof(header));

	// Supports HG2 from Happiness! and later
	BYTE header2[4];
	pclArc->Read(header2, sizeof(header2));
	if (memcmp(header2, "\x78\xDA", 2) != 0)
		pclArc->Seek(*(LPLONG)header - 4, FILE_CURRENT);
	else
		pclArc->Seek(-4, FILE_CURRENT);

	// Width, height, number of bits
	LONG width = *(LPLONG)&header[0x0C];
	LONG height = *(LPLONG)&header[0x10];
	WORD bpp = *(LPWORD)&header[0x14];

	// Get buffer size
	DWORD dwSrcSize1 = *(LPDWORD)&header[0x20];
	DWORD dwDstSize1 = *(LPDWORD)&header[0x24];
	DWORD dwSrcSize2 = *(LPDWORD)&header[0x28];
	DWORD dwDstSize2 = *(LPDWORD)&header[0x2C];
	DWORD dwDstSize = width * height * 4 * 2 + 256;

	// Ensure buffers exist
	YCMemory<BYTE> clmbtSrc1(dwSrcSize1);
	YCMemory<BYTE> clmbtDst1(dwDstSize1);
	YCMemory<BYTE> clmbtSrc2(dwSrcSize2);
	YCMemory<BYTE> clmbtDst2(dwDstSize2);
	YCMemory<BYTE> clmbtDst(dwDstSize);

	// zlib decompression
	pclArc->Read(&clmbtSrc1[0], dwSrcSize1);
	pclArc->Read(&clmbtSrc2[0], dwSrcSize2);

	CZlib zlib;
	zlib.Decompress(&clmbtDst1[0], &dwDstSize1, &clmbtSrc1[0], dwSrcSize1);
	zlib.Decompress(&clmbtDst2[0], &dwDstSize2, &clmbtSrc2[0], dwSrcSize2);

	// Decoding
	BYTE  abyTable[128] = {0};

	DWORD dwDst1 = 0;
	DWORD dwDst2a = 0;
	DWORD dwDst2b = 0;
	DWORD dwDst = 0;

	DWORD EAX = 0;
	DWORD EBX = 0;
	DWORD ECX = 0;
	DWORD EDX = 0;
	DWORD EDI = 0;
	DWORD ESI = 0;
	DWORD EBP = 0;

	DWORD SP[128];

	abyTable[0x44] = clmbtDst2[dwDst2b++];

	ECX = abyTable[0x40];
	EDX = abyTable[0x44];

	ESI = (EDX >> ECX) & 1;

	EAX++;
	EAX &= 0x07;

	abyTable[0x40] = EAX;

	SP[0] = ESI;


//-- 004049C0 ----------------------------------------------------------------------------

	EBP = 1;

	while (true)
	{
		ESI = abyTable[0x40];

		if (ESI == 0)
		{
			abyTable[0x44] = clmbtDst2[dwDst2b++];
		}

		ECX = abyTable[0x40];
		EDX = abyTable[0x44];

		EDX = (EDX >> ECX) & 1;

		ESI++;
		ESI &= 0x07;

		abyTable[0x40] = ESI;

		if (EDX != 0)
		{
			break;
		}

		EBP <<= 1;
	}

	EBX = EBP;
	ESI = 0;
	EBP >>= 1;

	if (EBP != 0)
	{
		while (true)
		{
			EDI = abyTable[0x40];

			ESI <<= 1;

			if (EDI == 0)
			{
				abyTable[0x44] = clmbtDst2[dwDst2b++];
			}

			ECX = abyTable[0x40];
			EDX = abyTable[0x44];

			EDX = (EDX >> ECX) & 0x01;

			EDI++;
			EDI &= 0x07;

			ESI |= EDX;

			EBP >>= 1;

			abyTable[0x40] = EDI;

			if (EBP == 0)
			{
				break;
			}
		}
	}

	EAX = ESI + EBX;

//----------------------------------------------------------------------------------------

	ESI = SP[0];
	EBX = EAX;
	ECX = EBX;
	EAX = 0;
	EDX = ECX;

	ECX >>= 2;

	EAX = abyTable[0x40];

	EDX = clmbtDst1[dwDst1++];

	clmbtDst[dwDst++] = EDX;

/*
	{
		LPBYTE pbuf2 = &buf2[0];
		DWORD tmp1 = 0;
		DWORD tmp2 = 0;

		DWORD EAX = tmp1;
		if (tmp1 == 0)
			tmp2 = *pbuf2++;
		DWORD CL = tmp1;
		DWORD DL = tmp2;
		DWORD ESI = DL;
		ESI >>= CL;
		ESI &= 1;
		EAX++;
		EAX &= 7;
		tmp1 = EAX;

		EAX = Decode1(&pbuf2, &tmp1, &tmp2);
	}
*/
	return TRUE;
}

//DWORD CWindmill::Sub(LPBYTE* pbuf2, LPDWORD tmp1, LPDWORD tmp2)
//{


//}