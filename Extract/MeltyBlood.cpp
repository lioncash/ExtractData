#include "stdafx.h"
#include "MeltyBlood.h"

// Function that gets file information from MELTY BLOOD *.data files
bool CMeltyBlood::Mount(CArcFile* pclArc)
{
	if ((pclArc->GetHed()[0] != 0x00) && (pclArc->GetHed()[0] != 0x01))
		return false;

	if ((pclArc->GetArcName().Left(5) != _T("data0")) || (pclArc->GetArcExten() != _T(".p")))
		return false;

	DWORD deckey = 0xE3DF59AC;

	// Get file count
	DWORD ctFile;
	pclArc->Seek(4, FILE_BEGIN);
	pclArc->Read(&ctFile, 4);
	ctFile ^= deckey;

	// Get index size from file count
	DWORD index_size = ctFile * 68;

	// Get index
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Read(pIndex, index_size);

	for (int i = 0; i < (int)ctFile; i++)
	{
		// Get file name
		TCHAR szFileName[60];
		memcpy(szFileName, pIndex, 60);
		
		// Decrypt filename
		for (int j = 0; j < 59; j++)
		{
			szFileName[j] ^= i * j * 3 + 61;
		}

		// Add to listview
		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.sizeOrg = *(LPDWORD)&pIndex[64] ^ deckey;
		infFile.sizeCmp = infFile.sizeOrg;
		infFile.start = *(LPDWORD)&pIndex[60];
		infFile.end = infFile.start + infFile.sizeOrg;
		infFile.title = _T("MeltyBlood");
		pclArc->AddFileInfo(infFile);

		pIndex += 68;
	}

	return true;
}

// Extraction function
bool CMeltyBlood::Decode(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if (file_info->format != _T("MeltyBlood"))
		return false;

	// Create output file
	pclArc->OpenFile();

	// Decodes data range 0x00`0x2173, Output
	Decrypt(pclArc);

	// Output earlier than 0x2173
	if (file_info->sizeOrg > 0x2173)
		pclArc->ReadWrite(file_info->sizeOrg - 0x2173);

	return true;
}

// Data decryption function
void CMeltyBlood::Decrypt(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	// Ensure buffer
	int lim = (file_info->sizeOrg < 0x2173) ? file_info->sizeOrg : 0x2173;
	YCMemory<BYTE> buf(lim);

	pclArc->Read(&buf[0], lim);

	// Decryption
	int keylen = (int) file_info->name.GetLength();
	
	for (int i = 0; i < lim; i++)
	{
		buf[i] ^= file_info->name[i % keylen] + i + 3;
	}

	pclArc->WriteFile(&buf[0], lim);
}
