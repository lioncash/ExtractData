#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Image.h"
#include "../Sound/Wav.h"
#include "../Sound/Ogg.h"
#include "Katakoi.h"

BOOL CKatakoi::Mount(CArcFile* pclArc)
{
	if (MountIar(pclArc))
		return TRUE;

	if (MountWar(pclArc))
		return TRUE;

	return FALSE;
}

bool CKatakoi::MountIar(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".iar"))
		return false;

	if (memcmp(pclArc->GetHed(), "iar ", 4) != 0)
		return false;

	// Version check
	DWORD dwVersion = *(LPDWORD)&pclArc->GetHed()[4];
	DWORD dwFileEntrySize = 0;

	if (dwVersion == 2)
	{
		dwFileEntrySize = 4;
	}
	else if (dwVersion == 3)
	{
		dwFileEntrySize = 8;
	}
	else
	{
		return false;
	}

	pclArc->SeekHed(0x1C);

	// Get number of files
	DWORD dwFiles;
	pclArc->Read(&dwFiles, 4);

	// Get index size
	DWORD dwIndexSize = dwFiles * dwFileEntrySize;

	// Get index
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// Get index filename
	YCMemory<BYTE> clmbtSec;
	DWORD dwNameIndex;

	const bool bSec = GetNameIndex(pclArc, clmbtSec, dwNameIndex);

	// File information retrieval
	TCHAR szFileName[_MAX_FNAME];
	TCHAR szWork[_MAX_FNAME];

	if (!bSec)
	{
		// Failed to get the filename index
		lstrcpy(szWork, pclArc->GetArcName());
		PathRemoveExtension(szWork);
	}

	for (DWORD i = 0; i < dwFiles; i++)
	{
		if (!bSec)
		{
			// Create a sequential filename
			_stprintf(szFileName, _T("%s_%06u"), szWork, i);
		}
		else
		{
			// Get the name of the file from the filename index
			lstrcpy(szFileName, (LPCTSTR)&clmbtSec[dwNameIndex]);

			dwNameIndex += strlen((char*)&clmbtSec[dwNameIndex]) + 1; // Filename
			dwNameIndex += strlen((char*)&clmbtSec[dwNameIndex]) + 1; // File type
			dwNameIndex += strlen((char*)&clmbtSec[dwNameIndex]) + 1; // Archive type
			dwNameIndex += 4 + *(LPDWORD)&clmbtSec[dwNameIndex];      // Archive name length + Archive name + File number
		}

		SFileInfo stfiWork;

		stfiWork.name = szFileName;
		stfiWork.start = *(LPDWORD)&clmbtIndex[i * dwFileEntrySize];
		stfiWork.end = ((i + 1) < dwFiles) ? *(LPDWORD)&clmbtIndex[(i+1) * dwFileEntrySize] : pclArc->GetArcSize();
		stfiWork.sizeCmp = stfiWork.end - stfiWork.start;
		stfiWork.sizeOrg = stfiWork.sizeCmp;

		pclArc->AddFileInfo(stfiWork);
	}

	return true;
}

bool CKatakoi::MountWar(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".war"))
		return false;

	if (memcmp(pclArc->GetHed(), "war ", 4) != 0)
		return false;

	// Version check
	DWORD dwVersion = *(LPDWORD)&pclArc->GetHed()[4];
	DWORD dwFileEntrySize = 0;

	if (dwVersion == 8)
	{
		dwFileEntrySize = 24;
	}
	else
	{
		return false;
	}

	pclArc->SeekHed(0x08);

	// Get the number of files
	DWORD dwFiles;
	pclArc->Read(&dwFiles, 4);

	// Get index size
	DWORD dwIndexSize = dwFiles * dwFileEntrySize;

	// Get index
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	pclArc->SeekCur(0x04);
	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// Get the filename index
	YCMemory<BYTE> clmbtSec;
	DWORD dwNameIndex;

	const bool bSec = GetNameIndex(pclArc, clmbtSec, dwNameIndex);

	// Set index for each archive filename to see if it is correct(Used in delta synthesis/decoding)
	pclArc->SetFlag(bSec);

	// Getting file info

	TCHAR szFileName[_MAX_FNAME];
	TCHAR szWork[_MAX_FNAME];

	if (!bSec)
	{
		// Failed to get the filename index
		lstrcpy(szWork, pclArc->GetArcName());
		PathRemoveExtension(szWork);
	}

	for (DWORD i = 0; i < dwFiles; i++)
	{
		if (!bSec)
		{
			// Create a sequential filename
			_stprintf(szFileName, _T("%s_%06u"), szWork, i);
		}
		else
		{
			// Get filename from the filename index
			lstrcpy(szFileName, (LPCTSTR)&clmbtSec[dwNameIndex]);

			dwNameIndex += strlen((char*)&clmbtSec[dwNameIndex]) + 1; // File name
			dwNameIndex += strlen((char*)&clmbtSec[dwNameIndex]) + 1; // File type
			dwNameIndex += strlen((char*)&clmbtSec[dwNameIndex]) + 1; // Archive type
			dwNameIndex += 4 + *(LPDWORD)&clmbtSec[dwNameIndex];      // Archive name length + Archive name + File number
		}

		SFileInfo stfiWork;
		stfiWork.name = szFileName;
		stfiWork.start = *(LPDWORD)&clmbtIndex[i * dwFileEntrySize];
		stfiWork.sizeCmp = *(LPDWORD)&clmbtIndex[i * dwFileEntrySize + 4];
		stfiWork.sizeOrg = stfiWork.sizeCmp;
		stfiWork.end = stfiWork.start + stfiWork.sizeCmp;
		pclArc->AddFileInfo(stfiWork);
	}

	return true;
}

bool CKatakoi::GetNameIndex(CArcFile* pclArc, YCMemory<BYTE>& clmbtSec, DWORD& dwNameIndex)
{
	// Open the filename represented by the index
	TCHAR szPathToSec[MAX_PATH];

	if (!GetPathToSec(szPathToSec, pclArc->GetArcPath()))
	{
		// sec5 file couldn't be found

//		MessageBox(pclArc->GetProg()->GetHandle(), _T("sec5ファイルが見つかりません。\nインストールフォルダ内にsec5ファイルが存在していない可能性があります。"), _T("エラー"), MB_OK);
		return false;
	}

	CFile clfSec;

	if (!clfSec.OpenForRead(szPathToSec))
	{
		// Failed to open the sec5 file

		return false;
	}

	DWORD dwSecSize = clfSec.GetFileSize();

	// Reading
	clmbtSec.resize(dwSecSize);
	clfSec.Read(&clmbtSec[0], dwSecSize);

	if (memcmp(&clmbtSec[0], "SEC5", 4) != 0)
	{
		// Incorrect sec5 file

		TCHAR szError[MAX_PATH * 2];

		_stprintf(szError, _T("%s is incorrect."), szPathToSec);
//		MessageBox(pclArc->GetProg()->GetHandle(), szError, _T("Error"), MB_OK);

		return false;
	}

	// Find the RESR

	for (dwNameIndex = 8; dwNameIndex < dwSecSize; )
	{
		if (memcmp(&clmbtSec[dwNameIndex], "RESR", 4) == 0)
		{
			// Found "RESR"

			DWORD dwNameIndexSize = *(LPDWORD)&clmbtSec[dwNameIndex + 4];
			DWORD dwNameIndexFiles = *(LPDWORD)&clmbtSec[dwNameIndex + 8];

			dwNameIndex += 12;

			// Find the index that matches the name of the archive

			for (DWORD i = 0; i < dwNameIndexFiles; i++)
			{
				DWORD dwWork = 0;
				dwWork += strlen((char*)&clmbtSec[dwNameIndex + dwWork]) + 1;	// File name
				dwWork += strlen((char*)&clmbtSec[dwNameIndex + dwWork]) + 1;	// File type
				dwWork += strlen((char*)&clmbtSec[dwNameIndex + dwWork]) + 1;	// Archive type

				DWORD dwLength = *(LPDWORD)&clmbtSec[dwNameIndex + dwWork];		// Archive name + File number
				dwWork += 4;

				for (DWORD j = (dwNameIndex + dwWork); ; j++)
				{
					if (clmbtSec[j] == '\0')
					{
						// Index dex doesn't match the name of the archive

						break;
					}

					if (lstrcmp((LPCTSTR)&clmbtSec[j], pclArc->GetArcName()) == 0)
					{
						// Found a match with the name of the archive

						// Validity

						if (lstrcmp(PathFindFileName(szPathToSec), _T("toa.sec5")) == 0)
						{
							// 杏奈ちゃんにお願い
							pclArc->SetFlag(true);
						}
						else if (lstrcmp(PathFindFileName(szPathToSec), _T("katakoi.sec5")) == 0)
						{
							// 片恋いの月
							pclArc->SetFlag(true);
						}

						return true;
					}
				}

				dwNameIndex += dwWork + dwLength;
			}
			break;
		}

		dwNameIndex += 8 + *(LPDWORD)&clmbtSec[dwNameIndex + 4];
	}

	// No file in the index was a match

//	MessageBox(pclArc->GetProg()->GetHandle(), _T("ファイル名の取得に失敗しました。\nアーカイブファイル名が変更されている可能性があります。"), _T("Error"), MB_OK);

	return false;
}

bool CKatakoi::GetPathToSec(LPTSTR pszPathToSec, const YCString& strPathToArc)
{
	TCHAR szWork[MAX_PATH];

	lstrcpy(szWork, strPathToArc);
	PathRemoveFileSpec(szWork);
	PathAppend(szWork, _T("*.sec5"));

	// Locate the sec5 file from within the archive folder.

	HANDLE          hFind;
	WIN32_FIND_DATA stwfdWork;

	hFind = FindFirstFile(szWork, &stwfdWork);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		// Locate the sec5 file from the installation folder (hopefully)

		PathRemoveFileSpec(szWork);
		PathRemoveFileSpec(szWork);
		PathAppend(szWork, _T("*.sec5"));

		hFind = FindFirstFile(szWork, &stwfdWork);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			// sec5 file couldn't be found

			return false;
		}
	}

	FindClose(hFind);

	lstrcpy(pszPathToSec, szWork);
	PathRemoveFileSpec(pszPathToSec);
	PathAppend(pszPathToSec, stwfdWork.cFileName);

	return true;
}

BOOL CKatakoi::Decode(CArcFile* pclArc)
{
	if (DecodeIar(pclArc))
		return TRUE;

	if (DecodeWar(pclArc))
		return TRUE;

	return FALSE;
}

bool CKatakoi::DecodeIar(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".iar"))
		return false;

	if (memcmp(pclArc->GetHed(), "iar ", 4) != 0)
		return false;

	SFileInfo* pstfiWork = pclArc->GetOpenFileInfo();

	// Reading
	YCMemory<BYTE> clmbtSrc(pstfiWork->sizeCmp);
	pclArc->Read(&clmbtSrc[0], pstfiWork->sizeCmp);

	// Output buffer
	DWORD dwDstSize = *(LPDWORD)&clmbtSrc[8];
	YCMemory<BYTE> clmbtDst(dwDstSize * 2);

	// Decompression
	DecompImage(&clmbtDst[0], dwDstSize, &clmbtSrc[64], *(LPDWORD)&clmbtSrc[16]);

	long lWidth = *(LPLONG)&clmbtSrc[32];
	long lHeight = *(LPLONG)&clmbtSrc[36];
	WORD wBpp;

	switch (*(LPBYTE)&clmbtSrc[0])
	{
		case 0x02:
			wBpp = 8;
			break;
		case 0x1C:
			wBpp = 24;
			break;
		case 0x3C:
			wBpp = 32;
			break;
		default:
			return false;
	}

	BOOL bDiff = (*(LPBYTE)&clmbtSrc[1] == 8);

	if (bDiff)
	{
		// Difference file
		DecodeCompose(pclArc, &clmbtDst[0], dwDstSize, lWidth, lHeight, wBpp);
	}
	else
	{
		CImage image;
		image.Init(pclArc, lWidth, lHeight, wBpp);
		image.WriteReverse(&clmbtDst[0], dwDstSize);
	}

	return true;
}

bool CKatakoi::DecodeWar(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".war"))
		return false;

	if (memcmp(pclArc->GetHed(), "war ", 4) != 0)
		return false;

	SFileInfo* pstfiWork = pclArc->GetOpenFileInfo();

	// Reading
	YCMemory<BYTE> clmbtSrc(pstfiWork->sizeCmp);
	pclArc->Read(&clmbtSrc[0], pstfiWork->sizeCmp);

	if (memcmp(&clmbtSrc[0], "OggS", 4) == 0)
	{
		// Ogg Vorbis

		COgg ogg;
		ogg.Decode(pclArc, &clmbtSrc[0]);
	}
	else
	{
		// WAV (supposedly)

		DWORD dwDataSize = *(LPDWORD)&clmbtSrc[4];
		DWORD dwFreq = *(LPDWORD)&clmbtSrc[12];
		WORD  wChannels = *(LPWORD)&clmbtSrc[10];
		WORD  wBits = *(LPWORD)&clmbtSrc[22];

		CWav wav;
		wav.Init(pclArc, dwDataSize, dwFreq, wChannels, wBits);
		wav.Write(&clmbtSrc[24]);
	}

	return true;
}

void CKatakoi::GetBit(LPBYTE& pbySrc, DWORD& dwFlags)
{
	dwFlags >>= 1;

	if (dwFlags <= 0xFFFF)
	{
		// Less than or equal to 0xFFFF

		dwFlags = *(LPWORD)&pbySrc[0] | 0xFFFF0000;
		pbySrc += 2;
	}
}

bool CKatakoi::DecompImage(LPBYTE pbyDst, DWORD dwDstSize, LPBYTE pbySrc, DWORD dwSrcSize)
{
	DWORD dwFlags = 0; // Flag can always be initialized
	DWORD dwBack;
	DWORD dwLength;
	DWORD dwWork;
	LPBYTE pbyDstBegin = pbyDst;
	LPBYTE pbySrcEnd = pbySrc + dwSrcSize;
	LPBYTE pbyDstEnd = pbyDst + dwDstSize;

	while ((pbySrc < pbySrcEnd) && (pbyDst < pbyDstEnd))
	{
		GetBit(pbySrc, dwFlags);

		// Uncompressed data
		if (dwFlags & 1)
		{
			*pbyDst++ = *pbySrc++;
		}
		else // Compressed data
		{
			GetBit(pbySrc, dwFlags);

			if (dwFlags & 1)
			{
				// Compression pattern 1 (3 or more compressed bytes)

				// Determine the number of bytes to return
				GetBit(pbySrc, dwFlags);

				// Plus one byte

				dwWork = 1;
				dwBack = dwFlags & 1;

				GetBit(pbySrc, dwFlags);

				if ((dwFlags & 1) == 0)
				{
					// Plus 0x201 bytes
					GetBit(pbySrc, dwFlags);
					dwWork = 0x201;

					if ((dwFlags & 1) == 0)
					{
						// Plus 0x401 bytes

						GetBit(pbySrc, dwFlags);

						dwWork = 0x401;
						dwBack = (dwBack << 1) | (dwFlags & 1);

						GetBit(pbySrc, dwFlags);

						if ((dwFlags & 1) == 0)
						{
							// Plus 0x801 bytes

							GetBit(pbySrc, dwFlags);

							dwWork = 0x801;
							dwBack = (dwBack << 1) | (dwFlags & 1);

							GetBit(pbySrc, dwFlags);

							if ((dwFlags & 1) == 0)
							{
								// Plus 0x1001 bytes

								GetBit(pbySrc, dwFlags);

								dwWork = 0x1001;
								dwBack = (dwBack << 1) | (dwFlags & 1);
							}
						}
					}
				}

				dwBack = ((dwBack << 8) | *pbySrc++) + dwWork;

				// Determine the number of compressed bytes
				GetBit(pbySrc, dwFlags);

				if (dwFlags & 1)
				{
					// 3 bytes of compressed data
					dwLength = 3;
				}
				else
				{
					GetBit(pbySrc, dwFlags);

					if (dwFlags & 1)
					{
						// 4 bytes of compressed data
						dwLength = 4;
					}
					else
					{
						GetBit(pbySrc, dwFlags);

						if (dwFlags & 1)
						{
							// 5 bytes of compressed data
							dwLength = 5;
						}
						else
						{
							GetBit(pbySrc, dwFlags);

							if (dwFlags & 1)
							{
								// 6 bytes of compressed data
								dwLength = 6;
							}
							else
							{
								GetBit(pbySrc, dwFlags);

								if (dwFlags & 1)
								{
									// 7~8 bytes of compressed data
									GetBit(pbySrc, dwFlags);

									dwLength = (dwFlags & 1);
									dwLength += 7;
								}
								else
								{
									GetBit(pbySrc, dwFlags);

									if (dwFlags & 1)
									{
										// More than 17 bytes of compressed data
										dwLength = *pbySrc++ + 0x11;
									}
									else
									{
										// 9~16 bytes of compressed data
										GetBit(pbySrc, dwFlags);
										dwLength = (dwFlags & 1) << 2;

										GetBit(pbySrc, dwFlags);
										dwLength |= (dwFlags & 1) << 1;

										GetBit(pbySrc, dwFlags);
										dwLength |= (dwFlags & 1);

										dwLength += 9;
									}
								}
							}
						}
					}
				}
			}
			else
			{
				// Compression pattern 2 (Compressed data is 2 bytes)

				dwLength = 2;

				// Determine the number of bytes to return

				GetBit(pbySrc, dwFlags);

				if (dwFlags & 1)
				{
					GetBit(pbySrc, dwFlags);
					dwBack = (dwFlags & 1) << 0x0A;

					GetBit(pbySrc, dwFlags);
					dwBack |= (dwFlags & 1) << 0x09;

					GetBit(pbySrc, dwFlags);
					dwBack |= (dwFlags & 1) << 0x08;

					dwBack |= *pbySrc++;
					dwBack += 0x100;
				}
				else
				{
					dwBack = *pbySrc++ + 1;

					if (dwBack == 0x100)
					{
						// Exit

						break;
					}
				}
			}

			// Decompress compressed files

			if (dwBack > (pbyDst - pbyDstBegin))
			{
				return false;
			}

			LPBYTE pbyWorkOfDst = pbyDst - dwBack;

			for (DWORD k = 0; (k < dwLength) && (pbyDst < pbyDstEnd) && (pbyWorkOfDst < pbyDstEnd); k++)
			{
				*pbyDst++ = *pbyWorkOfDst++;
			}
		}
	}

	return true;
}

bool CKatakoi::DecodeCompose(CArcFile* pclArc, LPBYTE pbyDiff, DWORD dwDiffSize, long lWidthForDiff, long lHeightForDiff, WORD wBppForDiff)
{
	SFileInfo*       pstfiDiff = pclArc->GetOpenFileInfo();
	DWORD            dwDiffNum = pclArc->GetOpenFileInfoNum();

	const SFileInfo* pstfiBase = nullptr;
	BOOL             bExistsForBase = FALSE;
	TCHAR            szFileNameForBase[MAX_PATH];

	lstrcpy(szFileNameForBase, pstfiDiff->name);

	LPTSTR pszFileNumberFordiff1 = &szFileNameForBase[lstrlen(szFileNameForBase) - 1];
	LPTSTR pszFileNumberFordiff2 = &szFileNameForBase[lstrlen(szFileNameForBase) - 2];

	// Convert numerical value to a serial number
	long lFileNumberForDiff1 = _tcstol(pszFileNumberFordiff1, nullptr, 10);
	long lFileNumberForDiff2 = _tcstol(pszFileNumberFordiff2, nullptr, 10);

	if (pclArc->GetFlag())
	{
		// Base file search(Search from delta file)

		long lFileNumberForBase = lFileNumberForDiff1;
		long lCount = lFileNumberForDiff1;

		while (!bExistsForBase)
		{
			lFileNumberForBase--;
			lCount--;

			if (lCount < 0)
			{
				// End search
				break;
			}

			_stprintf(pszFileNumberFordiff1, _T("%d"), lFileNumberForBase);

			pstfiBase = pclArc->GetFileInfo(szFileNameForBase);

			if (pstfiBase == nullptr)
			{
				// Missing number file
				continue;
			}

			BYTE byWork;
			pclArc->SeekHed(pstfiBase->start + 1);
			pclArc->Read(&byWork, 1);

			if (byWork == 0)
			{
				// Found base file
				bExistsForBase = TRUE;
			}
		}

		// Base file search(Find difference after the file)

		lFileNumberForBase = lFileNumberForDiff1;
		lCount = lFileNumberForDiff1;

		while (!bExistsForBase)
		{
			lFileNumberForBase++;
			lCount++;

			if (lCount >= 10)
			{
				// End search
				break;
			}

			_stprintf(pszFileNumberFordiff1, _T("%d"), lFileNumberForBase);

			pstfiBase = pclArc->GetFileInfo(szFileNameForBase);

			if (pstfiBase == nullptr)
			{
				// Missing number file
				continue;
			}

			BYTE byWork;
			pclArc->SeekHed(pstfiBase->start + 1);
			pclArc->Read(&byWork, 1);

			if (byWork == 0)
			{
				// Found base file
				bExistsForBase = TRUE;
			}
		}

		// Base file search(2桁目を1つ戻して検索)

		lFileNumberForBase = (lFileNumberForDiff2 / 10) * 10;
		lCount = 10;

		while (!bExistsForBase)
		{
			lFileNumberForBase--;
			lCount--;

			if (lCount < 0)
			{
				// End search
				break;
			}

			_stprintf(pszFileNumberFordiff2, _T("%02d"), lFileNumberForBase);

			pstfiBase = pclArc->GetFileInfo(szFileNameForBase);

			if (pstfiBase == nullptr)
			{
				// Missing number file
				continue;
			}

			BYTE byWork;
			pclArc->SeekHed(pstfiBase->start + 1);
			pclArc->Read(&byWork, 1);

			if (byWork == 0)
			{
				// Found base file
				bExistsForBase = TRUE;
			}
		}
	}

	if (bExistsForBase)
	{
		// Base file exists
		YCMemory<BYTE> clmbtSrcForBase(pstfiBase->sizeCmp);
		pclArc->SeekHed(pstfiBase->start);
		pclArc->Read(&clmbtSrcForBase[0], pstfiBase->sizeCmp);

		long lWidthForBase = *(LPLONG)&clmbtSrcForBase[32];
		long lHeightForBase = *(LPLONG)&clmbtSrcForBase[36];

		if ((lWidthForBase >= lWidthForDiff) && (lHeightForBase >= lHeightForDiff))
		{
			// Large base
			DWORD          dwDstSizeForBase = *(LPDWORD)&clmbtSrcForBase[8];
			YCMemory<BYTE> clmbtDstForBase(dwDstSizeForBase);

			// Decompress base file
			DecompImage(&clmbtDstForBase[0], dwDstSizeForBase, &clmbtSrcForBase[64], *(LPDWORD)&clmbtSrcForBase[16]);

			// Synthesize base file and delta file
			Compose(&clmbtDstForBase[0], dwDstSizeForBase, pbyDiff, dwDiffSize, lWidthForBase, lWidthForDiff, wBppForDiff);

			// Output
			CImage cliWork;
			long   lWidth = *(LPLONG)&clmbtSrcForBase[32];
			long   lHeight = *(LPLONG)&clmbtSrcForBase[36];
			WORD   wBpp = wBppForDiff;

			cliWork.Init(pclArc, lWidth, lHeight, wBpp);
			cliWork.WriteReverse(&clmbtDstForBase[0], dwDstSizeForBase);

			return true;
		}
		else if ((lWidthForDiff >= lWidthForBase) && (lHeightForDiff >= lHeightForBase))
		{
			// Difference is greater
			DWORD          dwDstSizeForBase = *(LPDWORD)&clmbtSrcForBase[8];
			YCMemory<BYTE> clmbtDstForBase(dwDstSizeForBase);

			// Decompress base file
			DecompImage(&clmbtDstForBase[0], dwDstSizeForBase, &clmbtSrcForBase[64], *(LPDWORD)&clmbtSrcForBase[16]);

			// The difference in the size of the memory allocation
			DWORD          dwDstSize = lWidthForDiff * lHeightForDiff * (wBppForDiff >> 3);
			YCMemory<BYTE> clmbtDst(dwDstSize);
			memset(&clmbtDst[0], 0, dwDstSize);

			// Align base file in the lower-right
			long   lX = lWidthForDiff - lWidthForBase;
			long   lY = lHeightForDiff - lHeightForBase;
			LPBYTE pbyDstForBase = &clmbtDstForBase[0];
			LPBYTE pbyDst = &clmbtDst[0];

			long   lGapForX = lX * (wBppForDiff >> 3);
			long   lLineForBase = lWidthForBase * (wBppForDiff >> 3);
			long   lLineForDiff = lWidthForDiff * (wBppForDiff >> 3);

			// Fit under the vertical position
			pbyDst += lY * lLineForDiff;

			for (long y = lY; y < lHeightForDiff; y++)
			{
				// According to the horizontal position to the right.
				pbyDst += lGapForX;

				memcpy(pbyDst, pbyDstForBase, lLineForBase);

				pbyDst += lLineForBase;
				pbyDstForBase += lLineForBase;
			}

			// Synthesize the base file and the delta file
			Compose(&clmbtDst[0], dwDstSize, pbyDiff, dwDiffSize, lWidthForDiff, lWidthForDiff, wBppForDiff);

			// Output
			CImage cliWork;
			long   lWidth = lWidthForDiff;
			long   lHeight = lHeightForDiff;
			WORD   wBpp = wBppForDiff;

			cliWork.Init(pclArc, lWidth, lHeight, wBpp);
			cliWork.WriteReverse(&clmbtDst[0], dwDstSize);

			return true;
		}
	}

	// Base file doesn't exist.
	long lWidth = lWidthForDiff;
	long lHeight = lHeightForDiff;
	WORD wBpp = wBppForDiff;

	// Prepare black data
	DWORD          dwDstSize = ((lWidth * (wBpp >> 3) + 3) & 0xFFFFFFFC) * lHeight;
	YCMemory<BYTE> clmbtDst(dwDstSize);
	memset(&clmbtDst[0], 0, dwDstSize);

	// Synthesize black data
	Compose(&clmbtDst[0], dwDstSize, pbyDiff, dwDiffSize, lWidth, lWidth, wBpp);

	CImage cliWork;
	cliWork.Init(pclArc, lWidth, lHeight, wBpp);
	cliWork.WriteReverse(&clmbtDst[0], dwDstSize);

	// Error message output
/*
	if (pclArc->GetFlag()) {
		TCHAR szError[1024];
		_stprintf(szError, _T("%s\n\n元画像が見つからなかったため、差分合成を行わずに出力しました。"), pstfiDiff->name);
		MessageBox(pclArc->GetProg()->GetHandle(), szError, _T("Info"), MB_OK);
	}
*/
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesizes the base image and the difference image.
//
// 十一寒月氏が作成・公開しているiarのソースコードを参考にして作成しました。

bool CKatakoi::Compose(LPBYTE pbyDst, DWORD dwDstSize, LPBYTE pbySrc, DWORD dwSrcSize, long lWidthForDst, long lWidthForSrc, WORD wBpp)
{
	WORD wColors = wBpp >> 3;
	DWORD dwLine = lWidthForSrc * wColors;
	DWORD dwHeight = *(LPDWORD)&pbySrc[8];

	DWORD dwGapForX = 0;

	if (lWidthForDst > lWidthForSrc)
	{
		dwGapForX = (lWidthForDst - lWidthForSrc) * wColors;
	}

	DWORD dwSrc = 12;
	DWORD dwDst = *(LPDWORD)&pbySrc[4] * (dwGapForX + dwLine);

	while ((dwHeight--) && (dwSrc < dwSrcSize))
	{
		for (DWORD x = 0; x < dwGapForX; x++)
		{
			pbyDst[dwDst++] = 0;
		}

		DWORD dwCount = *(LPWORD)&pbySrc[dwSrc];
		dwSrc += 2;

		DWORD dwOffset = 0;

		while (dwCount--)
		{
			dwOffset += *(LPWORD)&pbySrc[dwSrc] * wColors;
			dwSrc += 2;

			DWORD dwLength = *(LPWORD)&pbySrc[dwSrc] * wColors;
			dwSrc += 2;

			while (dwLength--)
			{
				pbyDst[dwDst + dwOffset++] = pbySrc[dwSrc++];

				if ((dwDst + dwOffset) >= dwDstSize)
				{
					return true;
				}

				if (dwSrc >= dwSrcSize)
				{
					return true;
				}
			}
		}

		dwDst += dwLine;
	}

	return true;
}
