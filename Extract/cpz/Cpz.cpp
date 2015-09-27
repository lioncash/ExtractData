#include "stdafx.h"
#include "../../Image.h"
#include "../../MD5.h"
#include "PB2A.h"
#include "PB3B.h"
#include "Cpz.h"

/// Mounting
///
/// @param pclArc Archive
///
BOOL CCpz::Mount(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".cpz"))
		return FALSE;

	if (MountCpz1(pclArc))
		return TRUE;

	if (MountCpz2(pclArc))
		return TRUE;

	if (MountCpz3(pclArc))
		return TRUE;

	if (MountCpz5(pclArc))
		return TRUE;

	return FALSE;
}

/// CPZ1 Mounting
///
/// @param pclArc Archive
///
BOOL CCpz::MountCpz1(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "CPZ1", 4) != 0)
	{
		return FALSE;
	}

	// Read header
	BYTE abtHeader[16];
	pclArc->Read(abtHeader, sizeof(abtHeader));
	DWORD dwFiles = *(DWORD*)&abtHeader[4];
	DWORD dwIndexSize = *(DWORD*)&abtHeader[8];

	// Get index
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	DWORD          dwIndexPtr = 0;
	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// Decrypt the index
	Decrypt1(&clmbtIndex[0], dwIndexSize);
	DWORD dwOffset = (dwIndexSize + 16);

	for (DWORD i = 0; i < dwFiles; i++)
	{
		TCHAR szFileName[256];
		lstrcpy(szFileName, (LPCTSTR)&clmbtIndex[dwIndexPtr + 24]);

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.sizeCmp = *(DWORD*)&clmbtIndex[dwIndexPtr + 4];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.start = *(DWORD*)&clmbtIndex[dwIndexPtr + 8] + dwOffset;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;
		pclArc->AddFileInfo(stFileInfo);

		dwIndexPtr += *(DWORD*)&clmbtIndex[dwIndexPtr + 0];
	}

	return TRUE;
}

/// CPZ2 Mounting
///
/// @param pclArc Archive
///
BOOL CCpz::MountCpz2(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "CPZ2", 4) != 0)
		return FALSE;

	// Read header
	BYTE abtHeader[20];
	pclArc->Read(abtHeader, sizeof(abtHeader));
	DWORD dwFiles = *(DWORD*)&abtHeader[4] ^ 0xE47C59F3;
	DWORD dwIndexSize = *(DWORD*)&abtHeader[8] ^ 0x3F71DE2A;
	DWORD dwKey = *(DWORD*)&abtHeader[16] ^ 0x77777777 ^ 0x37A9F45B;

	// Get index
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	DWORD dwIndexPtr = 0;
	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// Decrypt index
	Decrypt2(&clmbtIndex[0], dwIndexSize, dwKey);
	DWORD dwOffset = (dwIndexSize + 20);

	for (DWORD i = 0; i < dwFiles; i++)
	{
		TCHAR szFileName[256];
		lstrcpy(szFileName, (LPCTSTR)&clmbtIndex[dwIndexPtr + 24]);

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.sizeCmp = *(DWORD*)&clmbtIndex[dwIndexPtr + 4];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.start = *(DWORD*)&clmbtIndex[dwIndexPtr + 8] + dwOffset;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;
		stFileInfo.key = *(DWORD*)&clmbtIndex[dwIndexPtr + 20] ^ 0x796C3AFD;
		pclArc->AddFileInfo(stFileInfo);

		dwIndexPtr += *(DWORD*)&clmbtIndex[dwIndexPtr + 0];
	}

	return TRUE;
}

/// CPZ3 Mounting
///
/// @param pclArc Archive
///
BOOL CCpz::MountCpz3(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "CPZ3", 4) != 0)
	{
		return FALSE;
	}

	// Read header
	BYTE abtHeader[20];
	pclArc->Read(abtHeader, sizeof(abtHeader));

	DWORD dwFiles = *(DWORD*)&abtHeader[4] ^ 0x5E9C4F37;
	DWORD dwIndexSize = *(DWORD*)&abtHeader[8] ^ 0xF32AED17;
	DWORD dwKey = *(DWORD*)&abtHeader[16] ^ 0xDDDDDDDD ^ 0x7BF4A539;

	// Get index
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	DWORD dwIndexPtr = 0;
	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// Decrypt index
	Decrypt3(&clmbtIndex[0], dwIndexSize, dwKey);
	DWORD dwOffset = (dwIndexSize + 20);

	for (DWORD i = 0; i < dwFiles; i++)
	{
		TCHAR szFileName[_MAX_FNAME];
		lstrcpy(szFileName, (LPCTSTR)&clmbtIndex[dwIndexPtr + 24]);

		// ファイル数が多いので、フォルダ名を付けてフォルダ分割できるように変更
		// プリミティブリンク用の処理なので、他タイトルで不具合出るかも
		YCString clsDirName;
		LPTSTR   pszDirNameEndPos = _tcschr(szFileName, _T('-'));

		if (pszDirNameEndPos != nullptr)
		{
			clsDirName.Append(szFileName, (pszDirNameEndPos + 3 - szFileName));
			clsDirName += _T("\\");
		}

		// Add to listview
		SFileInfo stFileInfo;
		stFileInfo.name = clsDirName + szFileName;
		stFileInfo.sizeCmp = *(DWORD*)&clmbtIndex[dwIndexPtr + 4];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.start = *(DWORD*)&clmbtIndex[dwIndexPtr + 8] + dwOffset;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;
		stFileInfo.key = *(DWORD*)&clmbtIndex[dwIndexPtr + 20] ^ 0xC7F5DA63;
		pclArc->AddFileInfo(stFileInfo);

		dwIndexPtr += *(DWORD*)&clmbtIndex[dwIndexPtr + 0];
	}

	return TRUE;
}

/// CPZ5 Mounting
///
/// @param pclArc Archive
///
BOOL CCpz::MountCpz5(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "CPZ5", 4) != 0)
		return FALSE;

	// Get header
	SCPZ5Header* pstHeader = (SCPZ5Header*)pclArc->GetHed();
	pclArc->SeekCur(sizeof(SCPZ5Header));

	// Decrypt header
	pstHeader->dwDirs ^= 0xFE3A53D9;
	pstHeader->dwTotalDirIndexSize ^= 0x37F298E7;
	pstHeader->dwTotalFileIndexSize ^= 0x7A6F3A2C;
	pstHeader->adwMD5[0] ^= 0x43DE7C19;
	pstHeader->adwMD5[1] ^= 0xCC65F415;
	pstHeader->adwMD5[2] ^= 0xD016A93C;
	pstHeader->adwMD5[3] ^= 0x97A3BA9A;
	pstHeader->dwIndexKey ^= 0xAE7D39BF;
	pstHeader->adwUnKnown2[0] ^= 0xFB73A955;
	pstHeader->adwUnKnown2[1] ^= 0x37ACF831;

	// Prepare MD5 data
	DWORD adwMD5Data[16];
	adwMD5Data[0] = pstHeader->adwMD5[0];
	adwMD5Data[1] = pstHeader->adwMD5[1];
	adwMD5Data[2] = pstHeader->adwMD5[2];
	adwMD5Data[3] = pstHeader->adwMD5[3];

	// Add padding
	CMD5 clMD5;
	clMD5.AppendPadding(adwMD5Data, 16, 48);

	// Set initial values
	DWORD adwInitMD5[4];
	adwInitMD5[0] = 0xC74A2B01;
	adwInitMD5[1] = 0xE7C8AB8F;
	adwInitMD5[2] = 0xD8BEDC4E;
	adwInitMD5[3] = 0x7302A4C5;

	// Calculate MD5
	SMD5 stMD5 = clMD5.Calculate(adwMD5Data, sizeof(adwMD5Data), adwInitMD5);
	pstHeader->adwMD5[0] = stMD5.adwABCD[3];
	pstHeader->adwMD5[1] = stMD5.adwABCD[1];
	pstHeader->adwMD5[2] = stMD5.adwABCD[2];
	pstHeader->adwMD5[3] = stMD5.adwABCD[0];

	// Read index
	DWORD          dwIndexSize = (pstHeader->dwTotalDirIndexSize + pstHeader->dwTotalFileIndexSize);
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// Decode entire directory index
	const BYTE* pbtDecryptTable;
	Decrypt5(&clmbtIndex[0], dwIndexSize, (pstHeader->dwIndexKey ^ 0x3795B39A));
	pbtDecryptTable = InitDecryptWithTable5(pstHeader->dwIndexKey, pstHeader->adwMD5[1]);
	DecryptWithTable5(&clmbtIndex[0], pstHeader->dwTotalDirIndexSize, pbtDecryptTable, 0x3A);

	// Set key
	DWORD adwKey[4];
	adwKey[0] = pstHeader->adwMD5[0] ^ (pstHeader->dwIndexKey + 0x76A3BF29);
	adwKey[1] = pstHeader->adwMD5[1] ^ pstHeader->dwIndexKey;
	adwKey[2] = pstHeader->adwMD5[2] ^ (pstHeader->dwIndexKey + 0x10000000);
	adwKey[3] = pstHeader->adwMD5[3] ^ pstHeader->dwIndexKey;

	// Decrypt the total directory index
	DWORD dwKeyPtr = 0;
	DWORD dwSeed = 0x76548AEF;
	BYTE* pbtWork = &clmbtIndex[0];

	for (DWORD i = 0; i < (pstHeader->dwTotalDirIndexSize >> 2); i++)
	{
		DWORD dwWork = (*(DWORD*)pbtWork ^ adwKey[dwKeyPtr++]) - 0x4A91C262;

		*(DWORD*)pbtWork = _lrotl(dwWork, 3) - dwSeed;

		dwKeyPtr &= 3;
		pbtWork += 4;
		dwSeed += 0x10FB562A;
	}

	for (DWORD i = 0; i < (pstHeader->dwTotalDirIndexSize & 3); i++)
	{
		*pbtWork++ = ((adwKey[dwKeyPtr++] >> 6) ^ *pbtWork) + 0x37;

		dwKeyPtr &= 3;
	}

	// Initialize decryption table
	pbtDecryptTable = InitDecryptWithTable5(pstHeader->dwIndexKey, pstHeader->adwMD5[2]);

	// Get file information
	BYTE* pbtCurrentDirIndex = &clmbtIndex[0];

	for (DWORD i = 0; i < pstHeader->dwDirs; i++)
	{
		BYTE* pbtNextDirIndex = pbtCurrentDirIndex + *(DWORD*)&pbtCurrentDirIndex[0];
		DWORD dwCurrentFileIndexOffset = *(DWORD*)&pbtCurrentDirIndex[8];
		DWORD dwNextFileIndexOffset;

		if ((i + 1) >= pstHeader->dwDirs)
		{
			// The last directory

			dwNextFileIndexOffset = pstHeader->dwTotalFileIndexSize;
		}
		else
		{
			dwNextFileIndexOffset = *(DWORD*)&pbtNextDirIndex[8];
		}

		// Decrypt the current file index
		DWORD dwCurrentFileIndexSize = (dwNextFileIndexOffset - dwCurrentFileIndexOffset);
		BYTE* pbtCurrentFileIndex = &clmbtIndex[pstHeader->dwTotalDirIndexSize + dwCurrentFileIndexOffset];
		DecryptWithTable5(pbtCurrentFileIndex, dwCurrentFileIndexSize, pbtDecryptTable, 0x7E);

		// Set key
		DWORD dwEntryKey = *(DWORD*)&pbtCurrentDirIndex[12];
		adwKey[0] = pstHeader->adwMD5[0] ^ dwEntryKey;
		adwKey[1] = pstHeader->adwMD5[1] ^ (dwEntryKey + 0x112233);
		adwKey[2] = pstHeader->adwMD5[2] ^ dwEntryKey;
		adwKey[3] = pstHeader->adwMD5[3] ^ (dwEntryKey + 0x34258765);

		// Decrypt current file index
		dwSeed = 0x2A65CB4E;
		pbtWork = pbtCurrentFileIndex;
		dwKeyPtr = 0;

		for (DWORD j = 0; (j < dwCurrentFileIndexSize >> 2); j++)
		{
			*(DWORD*)pbtWork = _lrotl((*(DWORD*)pbtWork ^ adwKey[dwKeyPtr++]) - dwSeed, 2) + 0x37A19E8B;

			dwKeyPtr &= 3;
			pbtWork += 4;
			dwSeed -= 0x139FA9B;
		}

		for (DWORD j = 0; j < (dwCurrentFileIndexSize & 3); j++)
		{
			*pbtWork++ = (*pbtWork ^ (BYTE)(adwKey[dwKeyPtr++] >> 4)) + 5;

			dwKeyPtr &= 3;
		}

		// Get file information
		BYTE* pbtFileEntry = pbtCurrentFileIndex;

		for (DWORD j = 0; j < *(DWORD*)&pbtCurrentDirIndex[4]; j++)
		{
			// Get file name
			TCHAR szFileName[_MAX_FNAME];
			if (strcmp((char*)&pbtCurrentDirIndex[16], "root") == 0)
			{
				_stprintf(szFileName, _T("%s"), &pbtFileEntry[24]);
			}
			else
			{
				_stprintf(szFileName, _T("%s\\%s"), &pbtCurrentDirIndex[16], &pbtFileEntry[24]);
			}

			// Additional file information
			SFileInfo stFileInfo;
			stFileInfo.name = szFileName;
			stFileInfo.start = *(UINT64*)&pbtFileEntry[4] + sizeof(SCPZ5Header)+dwIndexSize;
			stFileInfo.sizeCmp = *(DWORD*)&pbtFileEntry[12];
			stFileInfo.sizeOrg = stFileInfo.sizeCmp;
			stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;
			stFileInfo.key = (pstHeader->dwIndexKey ^ (*(DWORD*)&pbtCurrentDirIndex[12] + *(DWORD*)&pbtFileEntry[20])) + pstHeader->dwDirs + 0xA3D61785;
			pclArc->AddFileInfo(stFileInfo);

			// Go to the next file entry
			pbtFileEntry += *(DWORD*)&pbtFileEntry[0];
		}

		// Go to the next directory entry
		pbtCurrentDirIndex = pbtNextDirIndex;
	}

	return TRUE;
}

/// Decoding
///
/// @param pclArc Archive
///
BOOL CCpz::Decode(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".cpz"))
		return FALSE;

	if (DecodeCpz1(pclArc))
		return TRUE;

	if (DecodeCpz2(pclArc))
		return TRUE;

	if (DecodeCpz3(pclArc))
		return TRUE;

	if (DecodeCpz5(pclArc))
		return TRUE;

	return FALSE;
}

/// CPZ1 Decoding
///
/// @param pclArc Archive
///
BOOL CCpz::DecodeCpz1(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "CPZ1", 4) != 0)
		return FALSE;

	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	// Read CPZ1
	DWORD          dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmbtSrc(dwSrcSize);
	pclArc->Read(&clmbtSrc[0], dwSrcSize);

	// Decryption
	Decrypt1(&clmbtSrc[0], dwSrcSize);

	// Output
	if (pstFileInfo->format == _T("PB2"))
	{
		CPB2A clPB2A;
		clPB2A.Decode(pclArc, &clmbtSrc[0], dwSrcSize);
	}
	else if (pstFileInfo->format == _T("MSK"))
	{
		long  lWidth = *(long*)&clmbtSrc[8];
		long  lHeight = *(long*)&clmbtSrc[12];
		DWORD dwDstSize = (lWidth * lHeight);

		CImage clImage;
		clImage.Init(pclArc, lWidth, lHeight, 8);
		clImage.WriteReverse(&clmbtSrc[16], dwDstSize);
	}
	else
	{
		pclArc->OpenFile();
		pclArc->WriteFile(&clmbtSrc[0], dwSrcSize);
	}

	return TRUE;
}

/// CPZ2 Decoding
///
/// @param pclArc Archive
///
BOOL CCpz::DecodeCpz2(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "CPZ2", 4) != 0)
		return FALSE;

	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	// Read CPZ2
	DWORD          dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmbtSrc(dwSrcSize);
	pclArc->Read(&clmbtSrc[0], dwSrcSize);

	// Decryption
	Decrypt2(&clmbtSrc[0], dwSrcSize, pstFileInfo->key);

	// Output
	if (pstFileInfo->format == _T("PB2"))
	{
		CPB2A clPB2A;
		clPB2A.Decode(pclArc, &clmbtSrc[0], dwSrcSize);
	}
	else if (pstFileInfo->format == _T("MSK"))
	{
		long  lWidth = *(long*)&clmbtSrc[8];
		long  lHeight = *(long*)&clmbtSrc[12];
		DWORD dwDstSize = (lWidth * lHeight);

		CImage clImage;
		clImage.Init(pclArc, lWidth, lHeight, 8);
		clImage.WriteReverse(&clmbtSrc[16], dwDstSize);
	}
	else
	{
		pclArc->OpenFile();
		pclArc->WriteFile(&clmbtSrc[0], dwSrcSize);
	}

	return TRUE;
}

/// CPZ3 Decoding
///
/// @param pclArc Archive
///
BOOL CCpz::DecodeCpz3(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "CPZ3", 4) != 0)
		return FALSE;

	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	// Read CPZ3
	DWORD          dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmbtSrc(dwSrcSize);
	pclArc->Read(&clmbtSrc[0], dwSrcSize);

	// Decryption
	Decrypt3(&clmbtSrc[0], dwSrcSize, pstFileInfo->key);

	if (pstFileInfo->format == _T("PB3"))
	{
		CPB3B clPB3B;
		clPB3B.Decode(pclArc, &clmbtSrc[0], dwSrcSize, OnDecrypt3FromPB3B);
	}
	else
	{
		pclArc->OpenFile();
		pclArc->WriteFile(&clmbtSrc[0], dwSrcSize);
	}

	return TRUE;
}

/// CPZ5 Decoding
///
/// @param pclArc Archive
///
BOOL CCpz::DecodeCpz5(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "CPZ5", 4) != 0)
		return FALSE;

	SFileInfo*   pstFileInfo = pclArc->GetOpenFileInfo();
	SCPZ5Header* pstCPZ5Header = (SCPZ5Header*)pclArc->GetHed();

	// Read CPZ5
	DWORD          dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmbtSrc(dwSrcSize);
	pclArc->Read(&clmbtSrc[0], dwSrcSize);

	// Decryption
	const BYTE* pbtTable = InitDecryptWithTable5(pstCPZ5Header->adwMD5[3], pstCPZ5Header->dwIndexKey);
	DecryptOfData5(&clmbtSrc[0], dwSrcSize, pbtTable, pstCPZ5Header->adwMD5, pstFileInfo->key);

	if (pstFileInfo->format == _T("PB3"))
	{
		CPB3B clPB3B;
		clPB3B.Decode(pclArc, &clmbtSrc[0], dwSrcSize, OnDecrypt5FromPB3B);
	}
	else
	{
		pclArc->OpenFile();
		pclArc->WriteFile(&clmbtSrc[0], dwSrcSize);
	}

	return TRUE;
}

/// Decryption function 1
///
/// @param pbtTarget Decoded data
/// @param dwSize    Decoding size
///
void CCpz::Decrypt1(BYTE* pbtTarget, DWORD dwSize)
{
	static const BYTE abtCrypt[] = "掴乱跨虎規日壅諺庫誤絞豪股誇砧後口糊己交\x8B\xFE謙倖規弭胡綿戸湖候誇騎";

	for (DWORD i = 0; i < dwSize; i++)
	{
		pbtTarget[i] = (pbtTarget[i] ^ abtCrypt[i & 63]) - 0x6C;
	}
}

/// Decryption function 2
///
/// @param pbtTarget Decoded data
/// @param dwSize    Decoding size
/// @param dwKey     Key
///
void CCpz::Decrypt2(BYTE* pbtTarget, DWORD dwSize, DWORD dwKey)
{
	static const DWORD adwCrypt[] =
	{
		0x3A68CDBF, 0xD3C3A711, 0x8414876E, 0x657BEFDB,
		0xCDD7C125, 0x09328580, 0x288FFEDD, 0x99EBF13A,
		0x5A471F95, 0x1EA3F4F1, 0xF4FF524E, 0xD358E8A9,
		0xC5B71015, 0xA913046F, 0x2D6FD2BD, 0x68C8BE19
	};

	DWORD dwShift = dwKey ^ 0x50000000;
	for (int i = 0; i < 7; i++)
	{
		dwShift = (dwShift >> 4) ^ dwKey;
	}
	dwShift = (dwShift & 0x0F) + 8;

	DWORD dwTablePtr = 0;
	for (DWORD i = 0; i < (dwSize >> 2); i++)
	{
		DWORD dwWork = ((adwCrypt[dwTablePtr++] + dwKey) ^ *(DWORD*)pbtTarget) - 0x15C3E7;

		*(DWORD*)pbtTarget = (dwWork << (32 - dwShift)) | (dwWork >> dwShift);

		dwTablePtr &= 0x0F;
		pbtTarget += 4;
	}

	for (DWORD i = 0, j = 0; i < (dwSize & 3); i++, j += 4)
	{
		*pbtTarget++ = (((adwCrypt[dwTablePtr++] + dwKey) >> j) ^ *pbtTarget) + 0x37;

		dwTablePtr &= 0x0F;
	}
}

/// Decryption function 3
///
/// @param pbtTarget Decoded data
/// @param dwSize    Decoding size
/// @param dwKey     Key
///
void CCpz::Decrypt3(BYTE* pbtTarget, DWORD dwSize, DWORD dwKey)
{
	static const DWORD adwCrypt[] =
	{
		0x4D0D4A5E, 0xB3ABF3E1, 0x3C37336D, 0x86C3F5F3,
		0x7D4F9B89, 0x58D7DE11, 0x6367778D, 0xA5F34629,
		0x067FA4B5, 0xED0AE742, 0xB19450CC, 0xE7204A5A,
		0xD9AF04F5, 0x5D3B687F, 0xC1C7A6FD, 0xFC502289
	};

	DWORD adwTable[16];
	for (int i = 0; i < 16; i++)
	{
		adwTable[i] = adwCrypt[i] + dwKey;
	}

	DWORD dwShift = dwKey;
	for (int i = 0; i < 7; i++)
	{
		dwShift = (dwShift >> 4) ^ dwKey;
	}

	dwShift = ((dwShift ^ 0xFFFFFFFD) & 0x0F) + 8;

	DWORD dwTablePtr = 3;
	for (DWORD i = 0; i < (dwSize >> 2); i++)
	{
		DWORD dwWork = (adwTable[dwTablePtr++] ^ *(DWORD*)pbtTarget) + 0x6E58A5C2;

		*(DWORD*)pbtTarget = (dwWork << dwShift) | (dwWork >> (32 - dwShift));

		dwTablePtr &= 0x0F;
		pbtTarget += 4;
	}

	for (DWORD i = (dwSize & 3); i > 0; i--)
	{
		*pbtTarget++ = ((adwTable[dwTablePtr++] >> (i * 4)) ^ *pbtTarget) + 0x52;

		dwTablePtr &= 0x0F;
	}
}

/// Decryption function 5
///
/// @param pbtTarget Decoded data
/// @param dwSize    Decoding size
/// @param dwKey     Key
///
void CCpz::Decrypt5(BYTE* pbtTarget, DWORD dwSize, DWORD dwKey)
{
	static const DWORD adwCrypt[] =
	{
		0xCD90F089, 0xE982B782, 0xA282AB88, 0xCD82718E, 0x52838A83, 0xA882AA82, 0x7592648E, 0xB582AB82,
		0xE182BF82, 0xDC82A282, 0x4281B782, 0xED82F48E, 0xBF82EA82, 0xA282E182, 0xB782DC82, 0x6081E682,
		0xC6824181, 0xA482A282, 0xE082A982, 0xF48EA482, 0xBF82C182, 0xA282E182, 0xB582DC82, 0xF481BD82
	};

	DWORD adwTable[24];
	for (DWORD i = 0; i < 24; i++)
	{
		adwTable[i] = (adwCrypt[i] - dwKey);
	}

	DWORD dwShift = dwKey;
	for (DWORD i = 0; i < 3; i++)
	{
		dwShift = (dwShift >> 8) ^ dwKey;
	}
	dwShift = ((dwShift ^ 0xFFFFFFFB) & 0x0F) + 7;

	DWORD dwTablePtr = 5;
	for (DWORD i = 0; i < (dwSize >> 2); i++)
	{
		DWORD dwWork = (adwTable[dwTablePtr++] ^ *(DWORD*)pbtTarget) + 0x784C5962;

		*(DWORD*)pbtTarget = _lrotr(dwWork, dwShift) + 0x01010101;

		dwTablePtr %= 24;
		pbtTarget += 4;
	}

	for (DWORD i = (dwSize & 3); i > 0; i--)
	{
		*pbtTarget++ = ((adwTable[dwTablePtr++] >> (i * 4)) ^ *pbtTarget) - 0x79;

		dwTablePtr %= 24;
	}
}

/// Initialization of decryption function 5 with a table
///
/// @param dwKey   Key
/// @param dwSeed  Seed
///
const BYTE* CCpz::InitDecryptWithTable5(DWORD dwKey, DWORD dwSeed)
{
	static BYTE abtDecryptTable[256];

	for (DWORD i = 0; i < 256; i++)
	{
		abtDecryptTable[i] = (BYTE)i;
	}

	for (DWORD i = 0; i < 256; i++)
	{
		std::swap(abtDecryptTable[(dwKey >> 16) & 0xFF], abtDecryptTable[dwKey & 0xFF]);
		std::swap(abtDecryptTable[(dwKey >> 8) & 0xFF], abtDecryptTable[(dwKey >> 24) & 0xFF]);

		dwKey = dwSeed + _lrotr(dwKey, 2) * 0x1A743125;
	}

	return abtDecryptTable;
}

/// Decoding function 5 with a table
///
/// @param pbtTarget       Decoded data
/// @param dwSize          Decoding size
/// @param pbtDecryptTable Decryption table
/// @param dwKey           Key
///
void CCpz::DecryptWithTable5(BYTE* pbtTarget, DWORD dwSize, const BYTE* pbtDecryptTable, DWORD dwKey)
{
	for (DWORD i = 0; i < dwSize; i++)
	{
		pbtTarget[i] = pbtDecryptTable[pbtTarget[i] ^ dwKey];
	}
}

/// Decoding CPZ5 data
///
/// @param pbtTarget       Decoded data
/// @param dwSize          Decoding size
/// @param pbtDecryptTable Decryption table
/// @param dwKey           Key
/// @param dwSeed          Seed
///
void CCpz::DecryptOfData5(BYTE* pbtTarget, DWORD dwSize, const BYTE* pbtDecryptTable, const DWORD* pdwKey, DWORD dwSeed)
{
	static const DWORD adwCrypt[] =
	{
		0xCD90F089, 0xE982B782, 0xA282AB88, 0xCD82718E, 0x52838A83, 0xA882AA82, 0x7592648E, 0xB582AB82,
		0xE182BF82, 0xDC82A282, 0x4281B782, 0xED82F48E, 0xBF82EA82, 0xA282E182, 0xB782DC82, 0x6081E682,
		0xC6824181, 0xA482A282, 0xE082A982, 0xF48EA482, 0xBF82C182, 0xA282E182, 0xB582DC82, 0xF481BD82
	};

	static const BYTE* pbtCrypt = (const BYTE*)adwCrypt;

	DWORD adwTable[24];
	BYTE* pbtTable = (BYTE*)adwTable;
	DWORD dwKey = (pdwKey[1] >> 2);

	for (DWORD i = 0; i < 96; i++)
	{
		pbtTable[i] = pbtDecryptTable[pbtCrypt[i]] ^ (BYTE)dwKey;
	}

	for (DWORD i = 0; i < 24; i++)
	{
		adwTable[i] ^= dwSeed;
	}

	DWORD dwTablePtr = 9;
	dwKey = 0x2547A39E;

	for (DWORD i = 0; i < (dwSize >> 2); i++)
	{
		*(DWORD*)pbtTarget = pdwKey[dwKey & 3] ^ ((*(DWORD*)pbtTarget ^ adwTable[(dwKey >> 6) & 0x0F] ^ (adwTable[dwTablePtr++] >> 1)) - dwSeed);

		dwTablePtr &= 0x0F;
		dwKey += *(DWORD*)pbtTarget + dwSeed;

		pbtTarget += 4;
	}

	DecryptWithTable5(pbtTarget, (dwSize & 3), pbtDecryptTable, 0xBC);
}

/// Callback function 3 from PB3B
///
/// @param pbtTarget    Data
/// @param dwSize       Size
/// @param pclArc       Archive
/// @param rfstFileInfo File info
///
void CCpz::OnDecrypt3FromPB3B(BYTE* pbtTarget, DWORD dwTargetSize, CArcFile* pclArc, const SFileInfo& rfstFileInfo)
{
	Decrypt3(pbtTarget, dwTargetSize, rfstFileInfo.key);
}

/// Callback function 5 from PB3B
///
///
/// @param pbtTarget    Data
/// @param dwSize       Size
/// @param pclArc       Archive
/// @param rfstFileInfo File info
///
void CCpz::OnDecrypt5FromPB3B(BYTE* pbtTarget, DWORD dwTargetSize, CArcFile* pclArc, const SFileInfo& rfstFileInfo)
{
	const SCPZ5Header* pstCPZ5Header = (SCPZ5Header*)pclArc->GetHed();
	const BYTE*         pbtTable;

	pbtTable = InitDecryptWithTable5(pstCPZ5Header->adwMD5[3], pstCPZ5Header->dwIndexKey);

	DecryptOfData5(pbtTarget, dwTargetSize, pbtTable, pstCPZ5Header->adwMD5, rfstFileInfo.key);
}
