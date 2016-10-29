#include "stdafx.h"
#include "../Image.h"
#include "InnocentGrey.h"

bool CInnocentGrey::Mount(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".dat")) || (memcmp(pclArc->GetHed(), "PACKDAT.", 8) != 0))
		return false;

	// Get file count
	DWORD ctFile;
	pclArc->Seek(8, FILE_BEGIN);
	pclArc->Read(&ctFile, 4);

	// Get index size from file count
	DWORD index_size = ctFile * 48;

	// Get index
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Seek(4, FILE_CURRENT);
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
		infFile.sizeOrg = *(LPDWORD)&pIndex[40];
		infFile.sizeCmp = *(LPDWORD)&pIndex[44];
		infFile.end = infFile.start + infFile.sizeCmp;
		infFile.title = _T("InnocentGrey");
		pclArc->AddFileInfo(infFile);

		pIndex += 48;
	}

	return true;
}

bool CInnocentGrey::Decode(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if ((file_info->title != _T("InnocentGrey")) || (pclArc->GetArcExten() != _T(".dat")) || (file_info->format != _T("S")))
		return false;

	if (file_info->format == _T("BMP"))
	{
		// Read
		YCMemory<BYTE> buf(file_info->sizeOrg);
		pclArc->Read(&buf[0], file_info->sizeOrg);

		// Decryption
		for (DWORD i = 0; i < file_info->sizeOrg; i++)
		{
			buf[i] ^= 0xFF;
		}

		// Output
		CImage image;
		image.Init(pclArc, &buf[0]);
		image.Write(file_info->sizeOrg);
	}
	else
	{
		// Ensure Buffer
		DWORD BufSize = pclArc->GetBufSize();
		YCMemory<BYTE> buf(BufSize);

		// Create output file
		pclArc->OpenFile();

		for (DWORD WriteSize = 0; WriteSize != file_info->sizeOrg; WriteSize += BufSize)
		{
			// Get buffer size
			pclArc->SetBufSize(&BufSize, WriteSize);

			// Decode output
			pclArc->Read(&buf[0], BufSize);
			for (DWORD i = 0; i < BufSize; i++)
				buf[i] ^= 0xFF;
			pclArc->WriteFile(&buf[0], BufSize);
		}
	}

	return true;
}
