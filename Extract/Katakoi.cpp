#include "StdAfx.h"
#include "Extract/Katakoi.h"

#include "ArcFile.h"
#include "File.h"
#include "Image.h"
#include "Sound/Ogg.h"
#include "Sound/Wav.h"

bool CKatakoi::Mount(CArcFile* archive)
{
	if (MountIar(archive))
		return true;

	if (MountWar(archive))
		return true;

	return false;
}

bool CKatakoi::MountIar(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".iar"))
		return false;

	if (memcmp(archive->GetHeader(), "iar ", 4) != 0)
		return false;

	// Version check
	DWORD dwVersion = *(LPDWORD)&archive->GetHeader()[4];
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

	archive->SeekHed(0x1C);

	// Get number of files
	DWORD dwFiles;
	archive->Read(&dwFiles, 4);

	// Get index size
	DWORD dwIndexSize = dwFiles * dwFileEntrySize;

	// Get index
	std::vector<BYTE> clmbtIndex(dwIndexSize);
	archive->Read(clmbtIndex.data(), clmbtIndex.size());

	// Get index filename
	std::vector<BYTE> clmbtSec;
	u32 dwNameIndex;

	const bool bSec = GetNameIndex(archive, clmbtSec, dwNameIndex);

	// File information retrieval
	TCHAR szFileName[_MAX_FNAME];
	TCHAR szWork[_MAX_FNAME];

	if (!bSec)
	{
		// Failed to get the filename index
		lstrcpy(szWork, archive->GetArcName());
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
		stfiWork.end = ((i + 1) < dwFiles) ? *(LPDWORD)&clmbtIndex[(i+1) * dwFileEntrySize] : archive->GetArcSize();
		stfiWork.sizeCmp = stfiWork.end - stfiWork.start;
		stfiWork.sizeOrg = stfiWork.sizeCmp;

		archive->AddFileInfo(stfiWork);
	}

	return true;
}

bool CKatakoi::MountWar(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".war"))
		return false;

	if (memcmp(archive->GetHeader(), "war ", 4) != 0)
		return false;

	// Version check
	DWORD dwVersion = *(LPDWORD)&archive->GetHeader()[4];
	DWORD dwFileEntrySize = 0;

	if (dwVersion == 8)
	{
		dwFileEntrySize = 24;
	}
	else
	{
		return false;
	}

	archive->SeekHed(0x08);

	// Get the number of files
	DWORD dwFiles;
	archive->Read(&dwFiles, 4);

	// Get index size
	DWORD dwIndexSize = dwFiles * dwFileEntrySize;

	// Get index
	std::vector<BYTE> clmbtIndex(dwIndexSize);
	archive->SeekCur(0x04);
	archive->Read(clmbtIndex.data(), clmbtIndex.size());

	// Get the filename index
	std::vector<BYTE> clmbtSec;
	u32 dwNameIndex;

	const bool bSec = GetNameIndex(archive, clmbtSec, dwNameIndex);

	// Set index for each archive filename to see if it is correct(Used in delta synthesis/decoding)
	archive->SetFlag(bSec);

	// Getting file info

	TCHAR szFileName[_MAX_FNAME];
	TCHAR szWork[_MAX_FNAME];

	if (!bSec)
	{
		// Failed to get the filename index
		lstrcpy(szWork, archive->GetArcName());
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
		archive->AddFileInfo(stfiWork);
	}

	return true;
}

bool CKatakoi::GetNameIndex(CArcFile* archive, std::vector<u8>& sec, u32& name_index)
{
	// Open the filename represented by the index
	TCHAR szPathToSec[MAX_PATH];

	if (!GetPathToSec(szPathToSec, archive->GetArcPath()))
	{
		// sec5 file couldn't be found

//		MessageBox(archive->GetProg()->GetHandle(), _T("sec5ファイルが見つかりません。\nインストールフォルダ内にsec5ファイルが存在していない可能性があります。"), _T("エラー"), MB_OK);
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
	sec.resize(dwSecSize);
	clfSec.Read(sec.data(), sec.size());

	if (memcmp(sec.data(), "SEC5", 4) != 0)
	{
		// Incorrect sec5 file

		TCHAR szError[MAX_PATH * 2];

		_stprintf(szError, _T("%s is incorrect."), szPathToSec);
//		MessageBox(archive->GetProg()->GetHandle(), szError, _T("Error"), MB_OK);

		return false;
	}

	// Find the RESR

	for (name_index = 8; name_index < dwSecSize; )
	{
		if (memcmp(&sec[name_index], "RESR", 4) == 0)
		{
			// Found "RESR"

			DWORD dwNameIndexSize = *(LPDWORD)&sec[name_index + 4];
			DWORD dwNameIndexFiles = *(LPDWORD)&sec[name_index + 8];

			name_index += 12;

			// Find the index that matches the name of the archive

			for (DWORD i = 0; i < dwNameIndexFiles; i++)
			{
				DWORD dwWork = 0;
				dwWork += strlen((char*)&sec[name_index + dwWork]) + 1;	// File name
				dwWork += strlen((char*)&sec[name_index + dwWork]) + 1;	// File type
				dwWork += strlen((char*)&sec[name_index + dwWork]) + 1;	// Archive type

				DWORD dwLength = *(LPDWORD)&sec[name_index + dwWork];		// Archive name + File number
				dwWork += 4;

				for (DWORD j = (name_index + dwWork); ; j++)
				{
					if (sec[j] == '\0')
					{
						// Index dex doesn't match the name of the archive

						break;
					}

					if (lstrcmp((LPCTSTR)&sec[j], archive->GetArcName()) == 0)
					{
						// Found a match with the name of the archive

						// Validity

						if (lstrcmp(PathFindFileName(szPathToSec), _T("toa.sec5")) == 0)
						{
							// 杏奈ちゃんにお願い
							archive->SetFlag(true);
						}
						else if (lstrcmp(PathFindFileName(szPathToSec), _T("katakoi.sec5")) == 0)
						{
							// 片恋いの月
							archive->SetFlag(true);
						}

						return true;
					}
				}

				name_index += dwWork + dwLength;
			}
			break;
		}

		name_index += 8 + *(LPDWORD)&sec[name_index + 4];
	}

	// No file in the index was a match

//	MessageBox(archive->GetProg()->GetHandle(), _T("ファイル名の取得に失敗しました。\nアーカイブファイル名が変更されている可能性があります。"), _T("Error"), MB_OK);

	return false;
}

bool CKatakoi::GetPathToSec(LPTSTR sec_path, const YCString& archive_path)
{
	TCHAR szWork[MAX_PATH];

	lstrcpy(szWork, archive_path);
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

	lstrcpy(sec_path, szWork);
	PathRemoveFileSpec(sec_path);
	PathAppend(sec_path, stwfdWork.cFileName);

	return true;
}

bool CKatakoi::Decode(CArcFile* archive)
{
	if (DecodeIar(archive))
		return true;

	if (DecodeWar(archive))
		return true;

	return false;
}

bool CKatakoi::DecodeIar(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".iar"))
		return false;

	if (memcmp(archive->GetHeader(), "iar ", 4) != 0)
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Reading
	std::vector<BYTE> clmbtSrc(file_info->sizeCmp);
	archive->Read(clmbtSrc.data(), clmbtSrc.size());

	// Output buffer
	DWORD dwDstSize = *(LPDWORD)&clmbtSrc[8];
	std::vector<BYTE> clmbtDst(dwDstSize * 2);

	// Decompression
	DecompImage(clmbtDst.data(), dwDstSize, &clmbtSrc[64], *(LPDWORD)&clmbtSrc[16]);

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
		DecodeCompose(archive, clmbtDst.data(), dwDstSize, lWidth, lHeight, wBpp);
	}
	else
	{
		CImage image;
		image.Init(archive, lWidth, lHeight, wBpp);
		image.WriteReverse(clmbtDst.data(), dwDstSize);
	}

	return true;
}

bool CKatakoi::DecodeWar(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".war"))
		return false;

	if (memcmp(archive->GetHeader(), "war ", 4) != 0)
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Reading
	std::vector<BYTE> clmbtSrc(file_info->sizeCmp);
	archive->Read(clmbtSrc.data(), clmbtSrc.size());

	if (memcmp(clmbtSrc.data(), "OggS", 4) == 0)
	{
		// Ogg Vorbis

		COgg ogg;
		ogg.Decode(archive, clmbtSrc.data());
	}
	else
	{
		// WAV (supposedly)

		DWORD dwDataSize = *(LPDWORD)&clmbtSrc[4];
		DWORD dwFreq = *(LPDWORD)&clmbtSrc[12];
		WORD  wChannels = *(LPWORD)&clmbtSrc[10];
		WORD  wBits = *(LPWORD)&clmbtSrc[22];

		CWav wav;
		wav.Init(archive, dwDataSize, dwFreq, wChannels, wBits);
		wav.Write(&clmbtSrc[24]);
	}

	return true;
}

void CKatakoi::GetBit(const u8*& src, u32& flags)
{
	flags >>= 1;

	if (flags <= 0xFFFF)
	{
		// Less than or equal to 0xFFFF

		flags = *(LPWORD)&src[0] | 0xFFFF0000;
		src += 2;
	}
}

bool CKatakoi::DecompImage(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	u32 dwFlags = 0; // Flag can always be initialized
	DWORD dwBack;
	DWORD dwLength;
	DWORD dwWork;
	const u8* pbyDstBegin = dst;
	const u8* pbySrcEnd = src + src_size;
	const u8* pbyDstEnd = dst + dst_size;

	while ((src < pbySrcEnd) && (dst < pbyDstEnd))
	{
		GetBit(src, dwFlags);

		// Uncompressed data
		if (dwFlags & 1)
		{
			*dst++ = *src++;
		}
		else // Compressed data
		{
			GetBit(src, dwFlags);

			if (dwFlags & 1)
			{
				// Compression pattern 1 (3 or more compressed bytes)

				// Determine the number of bytes to return
				GetBit(src, dwFlags);

				// Plus one byte

				dwWork = 1;
				dwBack = dwFlags & 1;

				GetBit(src, dwFlags);

				if ((dwFlags & 1) == 0)
				{
					// Plus 0x201 bytes
					GetBit(src, dwFlags);
					dwWork = 0x201;

					if ((dwFlags & 1) == 0)
					{
						// Plus 0x401 bytes

						GetBit(src, dwFlags);

						dwWork = 0x401;
						dwBack = (dwBack << 1) | (dwFlags & 1);

						GetBit(src, dwFlags);

						if ((dwFlags & 1) == 0)
						{
							// Plus 0x801 bytes

							GetBit(src, dwFlags);

							dwWork = 0x801;
							dwBack = (dwBack << 1) | (dwFlags & 1);

							GetBit(src, dwFlags);

							if ((dwFlags & 1) == 0)
							{
								// Plus 0x1001 bytes

								GetBit(src, dwFlags);

								dwWork = 0x1001;
								dwBack = (dwBack << 1) | (dwFlags & 1);
							}
						}
					}
				}

				dwBack = ((dwBack << 8) | *src++) + dwWork;

				// Determine the number of compressed bytes
				GetBit(src, dwFlags);

				if (dwFlags & 1)
				{
					// 3 bytes of compressed data
					dwLength = 3;
				}
				else
				{
					GetBit(src, dwFlags);

					if (dwFlags & 1)
					{
						// 4 bytes of compressed data
						dwLength = 4;
					}
					else
					{
						GetBit(src, dwFlags);

						if (dwFlags & 1)
						{
							// 5 bytes of compressed data
							dwLength = 5;
						}
						else
						{
							GetBit(src, dwFlags);

							if (dwFlags & 1)
							{
								// 6 bytes of compressed data
								dwLength = 6;
							}
							else
							{
								GetBit(src, dwFlags);

								if (dwFlags & 1)
								{
									// 7~8 bytes of compressed data
									GetBit(src, dwFlags);

									dwLength = (dwFlags & 1);
									dwLength += 7;
								}
								else
								{
									GetBit(src, dwFlags);

									if (dwFlags & 1)
									{
										// More than 17 bytes of compressed data
										dwLength = *src++ + 0x11;
									}
									else
									{
										// 9~16 bytes of compressed data
										GetBit(src, dwFlags);
										dwLength = (dwFlags & 1) << 2;

										GetBit(src, dwFlags);
										dwLength |= (dwFlags & 1) << 1;

										GetBit(src, dwFlags);
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

				GetBit(src, dwFlags);

				if (dwFlags & 1)
				{
					GetBit(src, dwFlags);
					dwBack = (dwFlags & 1) << 0x0A;

					GetBit(src, dwFlags);
					dwBack |= (dwFlags & 1) << 0x09;

					GetBit(src, dwFlags);
					dwBack |= (dwFlags & 1) << 0x08;

					dwBack |= *src++;
					dwBack += 0x100;
				}
				else
				{
					dwBack = *src++ + 1;

					if (dwBack == 0x100)
					{
						// Exit

						break;
					}
				}
			}

			// Decompress compressed files

			if (dwBack > (dst - pbyDstBegin))
			{
				return false;
			}

			LPBYTE pbyWorkOfDst = dst - dwBack;

			for (DWORD k = 0; (k < dwLength) && (dst < pbyDstEnd) && (pbyWorkOfDst < pbyDstEnd); k++)
			{
				*dst++ = *pbyWorkOfDst++;
			}
		}
	}

	return true;
}

bool CKatakoi::DecodeCompose(CArcFile* archive, const u8* diff, size_t diff_size, long diff_width, long diff_height, u16 diff_bpp)
{
	const SFileInfo* pstfiDiff = archive->GetOpenFileInfo();

	const SFileInfo* pstfiBase = nullptr;
	BOOL             bExistsForBase = FALSE;
	TCHAR            szFileNameForBase[MAX_PATH];

	lstrcpy(szFileNameForBase, pstfiDiff->name);

	LPTSTR pszFileNumberFordiff1 = &szFileNameForBase[lstrlen(szFileNameForBase) - 1];
	LPTSTR pszFileNumberFordiff2 = &szFileNameForBase[lstrlen(szFileNameForBase) - 2];

	// Convert numerical value to a serial number
	long lFileNumberForDiff1 = _tcstol(pszFileNumberFordiff1, nullptr, 10);
	long lFileNumberForDiff2 = _tcstol(pszFileNumberFordiff2, nullptr, 10);

	if (archive->GetFlag())
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

			pstfiBase = archive->GetFileInfo(szFileNameForBase);

			if (pstfiBase == nullptr)
			{
				// Missing number file
				continue;
			}

			BYTE byWork;
			archive->SeekHed(pstfiBase->start + 1);
			archive->Read(&byWork, 1);

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

			pstfiBase = archive->GetFileInfo(szFileNameForBase);

			if (pstfiBase == nullptr)
			{
				// Missing number file
				continue;
			}

			BYTE byWork;
			archive->SeekHed(pstfiBase->start + 1);
			archive->Read(&byWork, 1);

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

			pstfiBase = archive->GetFileInfo(szFileNameForBase);

			if (pstfiBase == nullptr)
			{
				// Missing number file
				continue;
			}

			BYTE byWork;
			archive->SeekHed(pstfiBase->start + 1);
			archive->Read(&byWork, 1);

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
		std::vector<BYTE> clmbtSrcForBase(pstfiBase->sizeCmp);
		archive->SeekHed(pstfiBase->start);
		archive->Read(clmbtSrcForBase.data(), clmbtSrcForBase.size());

		long lWidthForBase = *(LPLONG)&clmbtSrcForBase[32];
		long lHeightForBase = *(LPLONG)&clmbtSrcForBase[36];

		if (lWidthForBase >= diff_width && lHeightForBase >= diff_height)
		{
			// Large base
			DWORD             dwDstSizeForBase = *(LPDWORD)&clmbtSrcForBase[8];
			std::vector<BYTE> clmbtDstForBase(dwDstSizeForBase);

			// Decompress base file
			DecompImage(clmbtDstForBase.data(), dwDstSizeForBase, &clmbtSrcForBase[64], *(LPDWORD)&clmbtSrcForBase[16]);

			// Synthesize base file and delta file
			Compose(clmbtDstForBase.data(), dwDstSizeForBase, diff, diff_size, lWidthForBase, diff_width, diff_bpp);

			// Output
			CImage cliWork;
			long   lWidth = *(LPLONG)&clmbtSrcForBase[32];
			long   lHeight = *(LPLONG)&clmbtSrcForBase[36];
			WORD   wBpp = diff_bpp;

			cliWork.Init(archive, lWidth, lHeight, wBpp);
			cliWork.WriteReverse(clmbtDstForBase.data(), dwDstSizeForBase);

			return true;
		}
		else if (diff_width >= lWidthForBase && diff_height >= lHeightForBase)
		{
			// Difference is greater
			DWORD             dwDstSizeForBase = *(LPDWORD)&clmbtSrcForBase[8];
			std::vector<BYTE> clmbtDstForBase(dwDstSizeForBase);

			// Decompress base file
			DecompImage(clmbtDstForBase.data(), clmbtDstForBase.size(), &clmbtSrcForBase[64], *(LPDWORD)&clmbtSrcForBase[16]);

			// The difference in the size of the memory allocation
			DWORD             dwDstSize = diff_width * diff_height * (diff_bpp >> 3);
			std::vector<BYTE> clmbtDst(dwDstSize);

			// Align base file in the lower-right
			long   lX = diff_width - lWidthForBase;
			long   lY = diff_height - lHeightForBase;
			LPBYTE pbyDstForBase = clmbtDstForBase.data();
			LPBYTE pbyDst = clmbtDst.data();

			long   lGapForX = lX * (diff_bpp >> 3);
			long   lLineForBase = lWidthForBase * (diff_bpp >> 3);
			long   lLineForDiff = diff_width * (diff_bpp >> 3);

			// Fit under the vertical position
			pbyDst += lY * lLineForDiff;

			for (long y = lY; y < diff_height; y++)
			{
				// According to the horizontal position to the right.
				pbyDst += lGapForX;

				memcpy(pbyDst, pbyDstForBase, lLineForBase);

				pbyDst += lLineForBase;
				pbyDstForBase += lLineForBase;
			}

			// Synthesize the base file and the delta file
			Compose(clmbtDst.data(), clmbtDst.size(), diff, diff_size, diff_width, diff_width, diff_bpp);

			// Output
			CImage cliWork;
			long   lWidth = diff_width;
			long   lHeight = diff_height;
			WORD   wBpp = diff_bpp;

			cliWork.Init(archive, lWidth, lHeight, wBpp);
			cliWork.WriteReverse(clmbtDst.data(), clmbtDst.size());

			return true;
		}
	}

	// Base file doesn't exist.
	long lWidth = diff_width;
	long lHeight = diff_height;
	WORD wBpp = diff_bpp;

	// Prepare black data
	DWORD             dwDstSize = ((lWidth * (wBpp >> 3) + 3) & 0xFFFFFFFC) * lHeight;
	std::vector<BYTE> clmbtDst(dwDstSize);

	// Synthesize black data
	Compose(clmbtDst.data(), clmbtDst.size(), diff, diff_size, lWidth, lWidth, wBpp);

	CImage cliWork;
	cliWork.Init(archive, lWidth, lHeight, wBpp);
	cliWork.WriteReverse(clmbtDst.data(), clmbtDst.size());

	// Error message output
/*
	if (archive->GetFlag()) {
		TCHAR szError[1024];
		_stprintf(szError, _T("%s\n\n元画像が見つからなかったため、差分合成を行わずに出力しました。"), pstfiDiff->name);
		MessageBox(archive->GetProg()->GetHandle(), szError, _T("Info"), MB_OK);
	}
*/
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesizes the base image and the difference image.
//
// 十一寒月氏が作成・公開しているiarのソースコードを参考にして作成しました。

bool CKatakoi::Compose(u8* dst, size_t dst_size, const u8* src, size_t src_size, long dst_width, long src_width, u16 bpp)
{
	WORD wColors = bpp >> 3;
	DWORD dwLine = src_width * wColors;
	DWORD dwHeight = *(LPDWORD)&src[8];

	DWORD dwGapForX = 0;

	if (dst_width > src_width)
	{
		dwGapForX = (dst_width - src_width) * wColors;
	}

	size_t src_idx = 12;
	size_t dst_idx = *(LPDWORD)&src[4] * (dwGapForX + dwLine);

	while (dwHeight-- && src_idx < src_size)
	{
		for (DWORD x = 0; x < dwGapForX; x++)
		{
			dst[dst_idx++] = 0;
		}

		DWORD dwCount = *(LPWORD)&src[src_idx];
		src_idx += 2;

		size_t offset = 0;

		while (dwCount--)
		{
			offset += *(LPWORD)&src[src_idx] * wColors;
			src_idx += 2;

			DWORD dwLength = *(LPWORD)&src[src_idx] * wColors;
			src_idx += 2;

			while (dwLength--)
			{
				dst[dst_idx + offset++] = src[src_idx++];

				if (dst_idx + offset >= dst_size)
				{
					return true;
				}

				if (src_idx >= src_size)
				{
					return true;
				}
			}
		}

		dst_idx += dwLine;
	}

	return true;
}
