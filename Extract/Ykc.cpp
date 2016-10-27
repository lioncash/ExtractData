#include "stdafx.h"
#include "../ExtractBase.h"
#include "Ykc.h"

BOOL CYkc::Mount(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "YKC001", 6) != 0)
		return FALSE;

	// Get the index size and the offset to the index
	DWORD dwIndexOffset;
	DWORD dwIndexSize;
	pclArc->SeekHed(0x10);
	pclArc->Read(&dwIndexOffset, 4);
	pclArc->Read(&dwIndexSize, 4);

	// Get the index
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	pclArc->SeekHed(dwIndexOffset);
	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// Get the offset of the filename, and the offset to the index file name
	DWORD dwFileNameIndexOffset = *(LPDWORD)&clmbtIndex[0];
	DWORD dwFileNameIndexSize = dwIndexOffset - dwFileNameIndexOffset;

	// Get the filename index
	YCMemory<BYTE> clmbtFileNameIndex(dwFileNameIndexSize);
	pclArc->SeekHed(dwFileNameIndexOffset);
	pclArc->Read(&clmbtFileNameIndex[0], dwFileNameIndexSize);

	// Get file information
	for (DWORD i = 0, j = 0; i < dwIndexSize; i += 20)
	{
		// Get the length of the filename
		DWORD dwFileNameLen = *(LPDWORD)&clmbtIndex[i + 4];

		// Get the filename
		TCHAR szFileName[_MAX_FNAME];
		lstrcpy(szFileName, (LPCTSTR)&clmbtFileNameIndex[j]);
		j += dwFileNameLen;

		// Get file information
		SFileInfo stfiWork;
		stfiWork.name = szFileName;
		stfiWork.start = *(LPDWORD)&clmbtIndex[i + 8];
		stfiWork.sizeCmp = *(LPDWORD)&clmbtIndex[i + 12];
		stfiWork.sizeOrg = stfiWork.sizeCmp;
		stfiWork.end = stfiWork.start + stfiWork.sizeCmp;

		pclArc->AddFileInfo(stfiWork);
	}

	return TRUE;
}

BOOL CYkc::Decode(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "YKC001", 6) != 0)
		return FALSE;

	if (DecodeYKS(pclArc))
		return TRUE;

	if (DecodeYKG(pclArc))
		return TRUE;

	return FALSE;
}

bool CYkc::DecodeYKS(CArcFile* pclArc)
{
	SFileInfo* pstfiWork = pclArc->GetOpenFileInfo();
	if (pstfiWork->format != _T("YKS"))
		return false;

	// Read the YKS file
	YCMemory<BYTE> clmbtSrc(pstfiWork->sizeCmp);
	pclArc->Read(&clmbtSrc[0], pstfiWork->sizeCmp);

	if (memcmp(&clmbtSrc[0], "YKS001", 6) == 0)
	{
		// YKS001

		// Get the offset of the text portion
		DWORD dwTextOffset = *(LPDWORD)&clmbtSrc[0x20];

		// Decode the text portion
		for (DWORD i = dwTextOffset; i < pstfiWork->sizeCmp; i++)
		{
			clmbtSrc[i] ^= 0xAA;
		}

		// Output
		pclArc->OpenScriptFile();
		pclArc->WriteFile(&clmbtSrc[0], pstfiWork->sizeCmp);
	}
	else
	{
		// Other
		pclArc->OpenFile();
		pclArc->WriteFile(&clmbtSrc[0], pstfiWork->sizeCmp);
	}

	return true;
}

bool CYkc::DecodeYKG(CArcFile* pclArc)
{
	SFileInfo* pstfiWork = pclArc->GetOpenFileInfo();

	if (pstfiWork->format != _T("YKG"))
		return false;

	// Read the YKG file
	YCMemory<BYTE> clmbtSrc(pstfiWork->sizeCmp);
	pclArc->Read(&clmbtSrc[0], pstfiWork->sizeCmp);

	if (memcmp(&clmbtSrc[0], "YKG000", 6) == 0)
	{
		// YKG000

		// Fix the PNG header
		memcpy(&clmbtSrc[0x41], "PNG", 3);

		// Output
		pclArc->OpenFile(_T(".png"));
		pclArc->WriteFile(&clmbtSrc[0x40], pstfiWork->sizeCmp - 0x40);
	}
	else
	{
		// Other
		pclArc->OpenFile();
		pclArc->WriteFile(&clmbtSrc[0], pstfiWork->sizeCmp);
	}

	return true;
}
