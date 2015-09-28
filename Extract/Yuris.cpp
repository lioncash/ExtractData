#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Arc/Zlib.h"
#include "Yuris.h"
#include "Utils/ArrayUtils.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Mounting

BOOL CYuris::Mount(CArcFile* pclArc)
{
	if (MountYPF(pclArc))
		return TRUE;

	if (MountYMV(pclArc))
		return TRUE;

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// YPF Mounting

BOOL CYuris::MountYPF(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".ypf"))
		return FALSE;

	if (memcmp(pclArc->GetHed(), "YPF", 3) != 0)
		return FALSE;

	static const BYTE fnameLenTable[256] =
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x0B, 0x0A, 0x09, 0x10, 0x13, 0x0E, 0x0F,
		0x0C, 0x19, 0x12, 0x0D, 0x14, 0x1B, 0x16, 0x17, 0x18, 0x11, 0x1A, 0x15, 0x1E, 0x1D, 0x1C, 0x1F,
		0x23, 0x21, 0x22, 0x20, 0x24, 0x25, 0x29, 0x27, 0x28, 0x26, 0x2A, 0x2B, 0x2F, 0x2D, 0x32, 0x2C,
		0x30, 0x31, 0x2E, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
		0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
		0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
		0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
		0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
		0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
		0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
		0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
		0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
		0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
		0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
	};

	static const BYTE abtNotUseWord[] =
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
		0x20, 0x22, 0x2A, 0x2C, 0x3A, 0x3B, 0x3C, 0x3E, 0x3F, 0x7C, 0x7F
	};

	// Get file count
	DWORD ctFile;
	pclArc->Seek(8, FILE_BEGIN);
	pclArc->Read(&ctFile, 4);

	// Get index size
	DWORD index_size;
	pclArc->Read(&index_size, 4);

	// Get index
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Seek(16, FILE_CURRENT);
	pclArc->Read(pIndex, index_size);

	// Decoding test
	static DWORD adwFileInfoLength[] = { 26, 18 };

	BOOL  bSuccess = FALSE;
	DWORD dwFileInfoLength = 0;
	BYTE  btKey1 = 0;

	for (int i = 0; i < ArrayUtils::ArraySize(adwFileInfoLength); i++)
	{
		DWORD dwCnt = 0;
		DWORD dwIndex = 0;
		btKey1 = 0;

		for (; dwCnt < ctFile; dwCnt++)
		{
			// Decoded value of the filename

			dwIndex += 4;

			if (dwIndex >= index_size)
			{
				break;
			}

			char szFileName[256];
			BYTE btLength = fnameLenTable[255 - pIndex[dwIndex]];

			dwIndex += 1;
			if (dwIndex >= index_size)
			{
				break;
			}

			if ((dwIndex + btLength) >= index_size)
			{
				break;
			}

			for (BYTE j = 0; (j < btLength) && (btKey1 == 0); j++)
			{
				szFileName[j] = pIndex[dwIndex + j] ^ 0xff;

				if (IsDBCSLeadByte(szFileName[j]))
				{
					// Double-byte character

					j++;
					continue;
				}

				for (int k = 0; k < ArrayUtils::ArraySize(abtNotUseWord); k++)
				{
					if (szFileName[j] == abtNotUseWord[k])
					{
						btKey1 = 0x40;
						break;
					}
				}
			}

			dwIndex += btLength + adwFileInfoLength[i];
		}

		if ((dwCnt == ctFile) && (dwIndex == index_size))
		{
			// Successful decoding
			bSuccess = TRUE;
			dwFileInfoLength = adwFileInfoLength[i];
			break;
		}
	}

	if (!bSuccess)
	{
		// Anomaly occurred
		return FALSE;
	}

	for (DWORD i = 0; i < ctFile; i++)
	{
		// Filename
		TCHAR szFileName[256];
		BYTE  btLength = fnameLenTable[255 - pIndex[4]];

		for (DWORD j = 0; j < btLength; j++)
		{
			szFileName[j] = pIndex[5 + j] ^ 0xFF ^ btKey1;
		}

		szFileName[btLength] = _T('\0');

		BYTE bCmp = pIndex[5 + btLength + 1];

		// Add to listview
		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.sizeOrg = *(LPDWORD)&pIndex[5 + btLength + 2];
		infFile.sizeCmp = *(LPDWORD)&pIndex[5 + btLength + 6];
		infFile.start = *(LPDWORD)&pIndex[5 + btLength + 10];
		infFile.end = infFile.start + infFile.sizeCmp;
		if (bCmp) infFile.format = _T("zlib");

		pclArc->AddFileInfo(infFile);

		pIndex += 5 + btLength + dwFileInfoLength;
	}

	return TRUE;
}

/// YMV Mounting
BOOL CYuris::MountYMV(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".ymv"))
		return FALSE;

	if (memcmp(pclArc->GetHed(), "YSMV", 4) != 0)
		return FALSE;

	pclArc->SeekHed(8);

	// Get file count
	DWORD dwFiles;
	pclArc->Read(&dwFiles, 4);
	pclArc->SeekCur(4);

	// Get index
	YCMemory<DWORD> clmIndexOfOffset(dwFiles);
	YCMemory<DWORD> clmIndexOfSize(dwFiles);
	pclArc->Read(&clmIndexOfOffset[0], (4 * dwFiles));
	pclArc->Read(&clmIndexOfSize[0], (4 * dwFiles));

	// Additional file information
	for (DWORD i = 0; i < dwFiles; i++)
	{
		SFileInfo stFileInfo;
		stFileInfo.name.Format(_T("%s_%06d.jpg"), pclArc->GetArcName().GetFileTitle(), i);
		stFileInfo.start = clmIndexOfOffset[i];
		stFileInfo.sizeCmp = clmIndexOfSize[i];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo(stFileInfo);
	}

	return TRUE;
}

/// Decoding
BOOL CYuris::Decode(CArcFile* pclArc)
{
	if (DecodeYMV(pclArc))
		return TRUE;

	return FALSE;
}

/// YMV Decoding
BOOL CYuris::DecodeYMV(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".ymv"))
		return FALSE;

	if (memcmp(pclArc->GetHed(), "YSMV", 4) != 0)
		return FALSE;

	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	// Reading
	DWORD dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmSrc(dwSrcSize);
	pclArc->Read(&clmSrc[0], dwSrcSize);

	// Decoding
	for (DWORD i = 0; i < dwSrcSize; i++)
	{
		BYTE btKey = ((i & 0x0F) + 16);

		clmSrc[i] ^= btKey;
	}

	// Output
	pclArc->OpenFile();
	pclArc->WriteFile(&clmSrc[0], dwSrcSize);
	pclArc->CloseFile();

	return TRUE;
}
