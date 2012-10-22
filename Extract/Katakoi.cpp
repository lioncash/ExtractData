#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Image.h"
#include "../Wav.h"
#include "../Ogg.h"
#include "Katakoi.h"

BOOL CKatakoi::Mount(CArcFile* pclArc)
{
	if (MountIar(pclArc) == TRUE)
		return TRUE;
	if (MountWar(pclArc) == TRUE)
		return TRUE;

	return FALSE;
}

BOOL CKatakoi::MountIar(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".iar"))
		return FALSE;

	if (memcmp(pclArc->GetHed(), "iar ", 4) != 0)
		return FALSE;

	// Version check
	DWORD dwVersion = *(LPDWORD)&pclArc->GetHed()[4];
	DWORD dwFileEntrySize = 0;

	if (dwVersion == 2) {
		dwFileEntrySize = 4;
	}
	else if (dwVersion == 3) {
		dwFileEntrySize = 8;
	}
	else {
		return FALSE;
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

	BOOL bSec = GetNameIndex(pclArc, clmbtSec, dwNameIndex);

	// File information retrieval

	TCHAR szFileName[_MAX_FNAME];
	TCHAR szWork[_MAX_FNAME];

	if (!bSec) {
		// Failed to get the filename index

		lstrcpy(szWork, pclArc->GetArcName());
		PathRemoveExtension(szWork);
	}

	for (DWORD i = 0; i < dwFiles; i++) {
		if (!bSec) {
			// Create a sequential filename
			_stprintf(szFileName, _T("%s_%06u"), szWork, i);
		}
		else {
			// Get the name of the file from the filename index
			lstrcpy(szFileName, (LPCTSTR)&clmbtSec[dwNameIndex]);

			dwNameIndex += strlen((char*)&clmbtSec[dwNameIndex]) + 1;	// Filename
			dwNameIndex += strlen((char*)&clmbtSec[dwNameIndex]) + 1;	// File type
			dwNameIndex += strlen((char*)&clmbtSec[dwNameIndex]) + 1;	// Archive type
			dwNameIndex += 4 + *(LPDWORD)&clmbtSec[dwNameIndex];		// Archive name length + Archive name + File number
		}

		SFileInfo stfiWork;

		stfiWork.name = szFileName;
		stfiWork.start = *(LPDWORD)&clmbtIndex[i * dwFileEntrySize];
		stfiWork.end = ((i + 1) < dwFiles) ? *(LPDWORD)&clmbtIndex[(i+1) * dwFileEntrySize] : pclArc->GetArcSize();
		stfiWork.sizeCmp = stfiWork.end - stfiWork.start;
		stfiWork.sizeOrg = stfiWork.sizeCmp;

		pclArc->AddFileInfo(stfiWork);
	}

	return TRUE;
}

BOOL CKatakoi::MountWar(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".war"))
		return FALSE;

	if (memcmp(pclArc->GetHed(), "war ", 4) != 0)
		return FALSE;

	// Version check
	DWORD dwVersion = *(LPDWORD)&pclArc->GetHed()[4];
	DWORD dwFileEntrySize = 0;

	if (dwVersion == 8) {
		dwFileEntrySize = 24;
	}
	else {
		return FALSE;
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

	BOOL bSec = GetNameIndex(pclArc, clmbtSec, dwNameIndex);

	// ファイル名インデックスが取得できたかどうかをアーカイブごとに設定(デコードの差分合成時に使用)
	pclArc->SetFlag(bSec);

	// Getting file info

	TCHAR szFileName[_MAX_FNAME];
	TCHAR szWork[_MAX_FNAME];

	if (!bSec) {
		// Failed to get the filename index

		lstrcpy(szWork, pclArc->GetArcName());
		PathRemoveExtension(szWork);
	}

	for (DWORD i = 0; i < dwFiles; i++) {
		if (!bSec) {
			// Create a sequential filename
			_stprintf(szFileName, _T("%s_%06u"), szWork, i);
		}
		else {
			// Get filename from the filename index
			lstrcpy(szFileName, (LPCTSTR)&clmbtSec[dwNameIndex]);

			dwNameIndex += strlen((char*)&clmbtSec[dwNameIndex]) + 1;	// File name
			dwNameIndex += strlen((char*)&clmbtSec[dwNameIndex]) + 1;	// File type
			dwNameIndex += strlen((char*)&clmbtSec[dwNameIndex]) + 1;	// Archive type
			dwNameIndex += 4 + *(LPDWORD)&clmbtSec[dwNameIndex];		// Archive name length + Archive name + File number
		}

		SFileInfo stfiWork;

		stfiWork.name = szFileName;
		stfiWork.start = *(LPDWORD)&clmbtIndex[i * dwFileEntrySize];
		stfiWork.sizeCmp = *(LPDWORD)&clmbtIndex[i * dwFileEntrySize + 4];
		stfiWork.sizeOrg = stfiWork.sizeCmp;
		stfiWork.end = stfiWork.start + stfiWork.sizeCmp;
		pclArc->AddFileInfo(stfiWork);
	}

	return TRUE;
}

BOOL CKatakoi::GetNameIndex(CArcFile* pclArc, YCMemory<BYTE>& clmbtSec, DWORD& dwNameIndex)
{
	// ファイル名インデックスファイルを開く

	TCHAR szPathToSec[MAX_PATH];

	if (GetPathToSec(szPathToSec, pclArc->GetArcPath()) == FALSE) {
		// sec5ファイルが見つからなかった

//		MessageBox(pclArc->GetProg()->GetHandle(), _T("sec5ファイルが見つかりません。\nインストールフォルダ内にsec5ファイルが存在していない可能性があります。"), _T("エラー"), MB_OK);
		return FALSE;
	}

	CFile clfSec;

	if (!clfSec.OpenForRead(szPathToSec)) {
		// sec5ファイルのオープンに失敗

		return FALSE;
	}

	DWORD dwSecSize = clfSec.GetFileSize();

	// 読み込む
	clmbtSec.resize(dwSecSize);
	clfSec.Read(&clmbtSec[0], dwSecSize);

	if (memcmp(&clmbtSec[0], "SEC5", 4) != 0) {
		// sec5ファイルが正しくない

		TCHAR szError[MAX_PATH * 2];

		_stprintf(szError, _T("%s が正しくありません。"), szPathToSec);
//		MessageBox(pclArc->GetProg()->GetHandle(), szError, _T("エラー"), MB_OK);

		return FALSE;
	}

	// RESRを探す

	for (dwNameIndex = 8; dwNameIndex < dwSecSize; ) {
		if (memcmp(&clmbtSec[dwNameIndex], "RESR", 4) == 0) {
			// "RESR"が見つかった

			DWORD dwNameIndexSize = *(LPDWORD)&clmbtSec[dwNameIndex + 4];
			DWORD dwNameIndexFiles = *(LPDWORD)&clmbtSec[dwNameIndex + 8];

			dwNameIndex += 12;

			// アーカイブ名と一致するインデックスを探す

			for (DWORD i = 0; i < dwNameIndexFiles; i++) {
				DWORD dwWork = 0;
				dwWork += strlen((char*)&clmbtSec[dwNameIndex + dwWork]) + 1;	// ファイル名
				dwWork += strlen((char*)&clmbtSec[dwNameIndex + dwWork]) + 1;	// ファイルタイプ
				dwWork += strlen((char*)&clmbtSec[dwNameIndex + dwWork]) + 1;	// アーカイブタイプ

				DWORD dwLength = *(LPDWORD)&clmbtSec[dwNameIndex + dwWork];		// アーカイブ名 + ファイル番号長
				dwWork += 4;

				for (DWORD i = (dwNameIndex + dwWork); ; i++) {
					if (clmbtSec[i] == '\0') {
						// アーカイブ名と一致するインデックスではない

						break;
					}
					if (lstrcmp((LPCTSTR)&clmbtSec[i], pclArc->GetArcName()) == 0) {
						// アーカイブ名と一致するインデックスが見つかった

						// 差分合成の有効性

						if (lstrcmp(PathFindFileName(szPathToSec), _T("toa.sec5")) == 0) {
							// 杏奈ちゃんにお願い
							pclArc->SetFlag(TRUE);
						}
						else if (lstrcmp(PathFindFileName(szPathToSec), _T("katakoi.sec5")) == 0) {
							// 片恋いの月
							pclArc->SetFlag(TRUE);
						}

						return TRUE;
					}
				}

				dwNameIndex += dwWork + dwLength;
			}

			break;
		}

		dwNameIndex += 8 + *(LPDWORD)&clmbtSec[dwNameIndex + 4];
	}

	// アーカイブ名と一致するインデックスが見つからなかった

//	MessageBox(pclArc->GetProg()->GetHandle(), _T("ファイル名の取得に失敗しました。\nアーカイブファイル名が変更されている可能性があります。"), _T("エラー"), MB_OK);

	return FALSE;
}

BOOL CKatakoi::GetPathToSec(LPTSTR pszPathToSec, const YCString& strPathToArc)
{
	TCHAR szWork[MAX_PATH];

	lstrcpy(szWork, strPathToArc);
	PathRemoveFileSpec(szWork);
	PathAppend(szWork, _T("*.sec5"));

	// アーカイブフォルダ内からsec5ファイルを探す

	HANDLE			hFind;
	WIN32_FIND_DATA	stwfdWork;

	hFind = FindFirstFile(szWork, &stwfdWork);

	if (hFind == INVALID_HANDLE_VALUE) {
		// インストールフォルダ内(と思われる)からsec5ファイルを探す

		PathRemoveFileSpec(szWork);
		PathRemoveFileSpec(szWork);
		PathAppend(szWork, _T("*.sec5"));

		hFind = FindFirstFile(szWork, &stwfdWork);

		if (hFind == INVALID_HANDLE_VALUE) {
			// sec5ファイルが見つからなかった

			return FALSE;
		}
	}

	FindClose(hFind);

	lstrcpy(pszPathToSec, szWork);
	PathRemoveFileSpec(pszPathToSec);
	PathAppend(pszPathToSec, stwfdWork.cFileName);

	return TRUE;
}

BOOL CKatakoi::Decode(CArcFile* pclArc)
{
	if (DecodeIar(pclArc) == TRUE)
		return TRUE;
	if (DecodeWar(pclArc) == TRUE)
		return TRUE;

	return FALSE;
}

BOOL CKatakoi::DecodeIar(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".iar"))
		return FALSE;

	if (memcmp(pclArc->GetHed(), "iar ", 4) != 0)
		return FALSE;

	SFileInfo* pstfiWork = pclArc->GetOpenFileInfo();

	// 読み込み
	YCMemory<BYTE> clmbtSrc(pstfiWork->sizeCmp);
	pclArc->Read(&clmbtSrc[0], pstfiWork->sizeCmp);

	// 出力バッファ
	DWORD dwDstSize = *(LPDWORD)&clmbtSrc[8];
	YCMemory<BYTE> clmbtDst(dwDstSize * 2);

	// 解凍
	DecompImage(&clmbtDst[0], dwDstSize, &clmbtSrc[64], *(LPDWORD)&clmbtSrc[16]);

	long lWidth = *(LPLONG)&clmbtSrc[32];
	long lHeight = *(LPLONG)&clmbtSrc[36];
	WORD wBpp;

	switch (*(LPBYTE)&clmbtSrc[0]) {
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
		return FALSE;
	}

	BOOL bDiff = (*(LPBYTE)&clmbtSrc[1] == 8);

	if (bDiff) {
		// 差分ファイル
		DecodeCompose(pclArc, &clmbtDst[0], dwDstSize, lWidth, lHeight, wBpp);
	}
	else {
		CImage image;
		image.Init(pclArc, lWidth, lHeight, wBpp);
		image.WriteReverse(&clmbtDst[0], dwDstSize);
	}

	return TRUE;
}

BOOL CKatakoi::DecodeWar(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".war"))
		return FALSE;

	if (memcmp(pclArc->GetHed(), "war ", 4) != 0)
		return FALSE;

	SFileInfo* pstfiWork = pclArc->GetOpenFileInfo();

	// 読み込み
	YCMemory<BYTE> clmbtSrc(pstfiWork->sizeCmp);
	pclArc->Read(&clmbtSrc[0], pstfiWork->sizeCmp);

	if (memcmp(&clmbtSrc[0], "OggS", 4) == 0) {
		// Ogg Vorbis

		COgg ogg;
		ogg.Decode(pclArc, &clmbtSrc[0]);
	}
	else {
		// WAV(と断定)

		DWORD	dwDataSize = *(LPDWORD)&clmbtSrc[4];
		DWORD	dwFreq = *(LPDWORD)&clmbtSrc[12];
		WORD	wChannels = *(LPWORD)&clmbtSrc[10];
		WORD	wBits = *(LPWORD)&clmbtSrc[22];

		CWav wav;
		wav.Init(pclArc, dwDataSize, dwFreq, wChannels, wBits);
		wav.Write(&clmbtSrc[24]);
	}

	return TRUE;
}

void CKatakoi::GetBit(LPBYTE& pbySrc, DWORD& dwFlags)
{
	dwFlags >>= 1;

	if (dwFlags <= 0xFFFF) {
		// 0xFFFF以下になった

		dwFlags = *(LPWORD)&pbySrc[0] | 0xFFFF0000;
		pbySrc += 2;
	}
}

BOOL CKatakoi::DecompImage(LPBYTE pbyDst, DWORD dwDstSize, LPBYTE pbySrc, DWORD dwSrcSize)
{
	DWORD dwFlags = 0;		// フラグは必ず初期化すること
	DWORD dwBack;
	DWORD dwLength;
	DWORD dwWork;
	LPBYTE pbyDstBegin = pbyDst;
	LPBYTE pbySrcEnd = pbySrc + dwSrcSize;
	LPBYTE pbyDstEnd = pbyDst + dwDstSize;

	while ((pbySrc < pbySrcEnd) && (pbyDst < pbyDstEnd)) {
		GetBit(pbySrc, dwFlags);

		if (dwFlags & 1) {
			// 無圧縮データ

			*pbyDst++ = *pbySrc++;
		}
		else {
			// 圧縮データ

			GetBit(pbySrc, dwFlags);

			if (dwFlags & 1) {
				// 圧縮パターン1(圧縮データが3バイト以上)

				// 戻るバイト数を求める

				GetBit(pbySrc, dwFlags);

				// 1バイトプラス

				dwWork = 1;
				dwBack = dwFlags & 1;

				GetBit(pbySrc, dwFlags);

				if ((dwFlags & 1) == 0) {
					// 0x201バイトプラス

					GetBit(pbySrc, dwFlags);

					dwWork = 0x201;

					if ((dwFlags & 1) == 0) {
						// 0x401バイトプラス

						GetBit(pbySrc, dwFlags);

						dwWork = 0x401;
						dwBack = (dwBack << 1) | (dwFlags & 1);

						GetBit(pbySrc, dwFlags);

						if ((dwFlags & 1) == 0) {
							// 0x801バイトプラス

							GetBit(pbySrc, dwFlags);

							dwWork = 0x801;
							dwBack = (dwBack << 1) | (dwFlags & 1);

							GetBit(pbySrc, dwFlags);

							if ((dwFlags & 1) == 0) {
								// 0x1001バイトプラス

								GetBit(pbySrc, dwFlags);

								dwWork = 0x1001;
								dwBack = (dwBack << 1) | (dwFlags & 1);
							}
						}
					}
				}

				dwBack = ((dwBack << 8) | *pbySrc++) + dwWork;

				// 圧縮バイト数を求める

				GetBit(pbySrc, dwFlags);

				if (dwFlags & 1) {
					// 3バイトの圧縮データ

					dwLength = 3;
				}
				else {
					GetBit(pbySrc, dwFlags);

					if (dwFlags & 1) {
						// 4バイトの圧縮データ

						dwLength = 4;
					}
					else {
						GetBit(pbySrc, dwFlags);

						if (dwFlags & 1) {
							// 5バイトの圧縮データ

							dwLength = 5;
						}
						else {
							GetBit(pbySrc, dwFlags);

							if (dwFlags & 1) {
								// 6バイトの圧縮データ

								dwLength = 6;
							}
							else {
								GetBit(pbySrc, dwFlags);

								if (dwFlags & 1) {
									// 7～8バイトの圧縮データ

									GetBit(pbySrc, dwFlags);

									dwLength = (dwFlags & 1);
									dwLength += 7;
								}
								else {
									GetBit(pbySrc, dwFlags);

									if (dwFlags & 1) {
										// 17バイト以上の圧縮データ

										dwLength = *pbySrc++ + 0x11;
									}
									else {
										// 9～16バイトの圧縮データ

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
			else {
				// 圧縮パターン2(圧縮データが2バイト)

				dwLength = 2;

				// 戻るバイト数を求める

				GetBit(pbySrc, dwFlags);

				if (dwFlags & 1) {
					GetBit(pbySrc, dwFlags);
					dwBack = (dwFlags & 1) << 0x0A;

					GetBit(pbySrc, dwFlags);
					dwBack |= (dwFlags & 1) << 0x09;

					GetBit(pbySrc, dwFlags);
					dwBack |= (dwFlags & 1) << 0x08;

					dwBack |= *pbySrc++;
					dwBack += 0x100;
				}
				else {
					dwBack = *pbySrc++ + 1;

					if (dwBack == 0x100) {
						// 終了

						break;
					}
				}
			}

			// 圧縮データの展開

			if (dwBack > (pbyDst - pbyDstBegin)) {
				return FALSE;
			}

			LPBYTE pbyWorkOfDst = pbyDst - dwBack;

			for (DWORD k = 0; (k < dwLength) && (pbyDst < pbyDstEnd) && (pbyWorkOfDst < pbyDstEnd); k++) {
				*pbyDst++ = *pbyWorkOfDst++;
			}
		}
	}

	return TRUE;
}

BOOL CKatakoi::DecodeCompose(CArcFile* pclArc, LPBYTE pbyDiff, DWORD dwDiffSize, long lWidthForDiff, long lHeightForDiff, WORD wBppForDiff)
{
	SFileInfo*			pstfiDiff = pclArc->GetOpenFileInfo();
	DWORD				dwDiffNum = pclArc->GetOpenFileInfoNum();

	const SFileInfo*		pstfiBase;
	BOOL				bExistsForBase = FALSE;
	TCHAR				szFileNameForBase[MAX_PATH];

	lstrcpy(szFileNameForBase, pstfiDiff->name);

	LPTSTR				pszFileNumberFordiff1 = &szFileNameForBase[lstrlen(szFileNameForBase) - 1];
	LPTSTR				pszFileNumberFordiff2 = &szFileNameForBase[lstrlen(szFileNameForBase) - 2];

	// 連番を数値に変換

	long				lFileNumberForDiff1 = _tcstol(pszFileNumberFordiff1, NULL, 10);
	long				lFileNumberForDiff2 = _tcstol(pszFileNumberFordiff2, NULL, 10);

	if (pclArc->GetFlag()) {
		// ベースファイルの検索(差分ファイルより前を検索)

		long		lFileNumberForBase = lFileNumberForDiff1;
		long		lCount = lFileNumberForDiff1;

		while (!bExistsForBase) {
			lFileNumberForBase--;
			lCount--;

			if (lCount < 0) {
				// 検索終了

				break;
			}

			_stprintf(pszFileNumberFordiff1, _T("%d"), lFileNumberForBase);

			pstfiBase = pclArc->GetFileInfo(szFileNameForBase);

			if (pstfiBase == NULL) {
				// 欠番ファイル

				continue;
			}

			BYTE		byWork;

			pclArc->SeekHed(pstfiBase->start + 1);
			pclArc->Read(&byWork, 1);

			if (byWork == 0) {
				// ベースファイルが見つかった

				bExistsForBase = TRUE;
			}
		}

		// ベースファイルの検索(差分ファイルより後を検索)

		lFileNumberForBase = lFileNumberForDiff1;
		lCount = lFileNumberForDiff1;

		while (!bExistsForBase) {
			lFileNumberForBase++;
			lCount++;

			if (lCount >= 10) {
				// 検索終了

				break;
			}

			_stprintf(pszFileNumberFordiff1, _T("%d"), lFileNumberForBase);

			pstfiBase = pclArc->GetFileInfo(szFileNameForBase);

			if (pstfiBase == NULL) {
				// 欠番ファイル

				continue;
			}

			BYTE		byWork;

			pclArc->SeekHed(pstfiBase->start + 1);
			pclArc->Read(&byWork, 1);

			if (byWork == 0) {
				// ベースファイルが見つかった

				bExistsForBase = TRUE;
			}
		}

		// ベースファイルの検索(2桁目を1つ戻して検索)

		lFileNumberForBase = (lFileNumberForDiff2 / 10) * 10;
		lCount = 10;

		while (!bExistsForBase) {
			lFileNumberForBase--;
			lCount--;

			if (lCount < 0) {
				// 検索終了

				break;
			}

			_stprintf(pszFileNumberFordiff2, _T("%02d"), lFileNumberForBase);

			pstfiBase = pclArc->GetFileInfo(szFileNameForBase);

			if (pstfiBase == NULL) {
				// 欠番ファイル

				continue;
			}

			BYTE		byWork;

			pclArc->SeekHed(pstfiBase->start + 1);
			pclArc->Read(&byWork, 1);

			if (byWork == 0) {
				// ベースファイルが見つかった

				bExistsForBase = TRUE;
			}
		}
	}

	if (bExistsForBase) {
		// ベースファイルが存在する

		YCMemory<BYTE>		clmbtSrcForBase(pstfiBase->sizeCmp);

		pclArc->SeekHed(pstfiBase->start);
		pclArc->Read(&clmbtSrcForBase[0], pstfiBase->sizeCmp);

		long		lWidthForBase = *(LPLONG)&clmbtSrcForBase[32];
		long		lHeightForBase = *(LPLONG)&clmbtSrcForBase[36];

		if ((lWidthForBase >= lWidthForDiff) && (lHeightForBase >= lHeightForDiff)) {
			// ベースのほうが大きい

			DWORD				dwDstSizeForBase = *(LPDWORD)&clmbtSrcForBase[8];
			YCMemory<BYTE>		clmbtDstForBase(dwDstSizeForBase);

			// ベースファイルの解凍

			DecompImage(&clmbtDstForBase[0], dwDstSizeForBase, &clmbtSrcForBase[64], *(LPDWORD)&clmbtSrcForBase[16]);

			// ベースファイルと差分ファイルを合成

			Compose(&clmbtDstForBase[0], dwDstSizeForBase, pbyDiff, dwDiffSize, lWidthForBase, lWidthForDiff, wBppForDiff);

			// 出力

			CImage				cliWork;
			long				lWidth = *(LPLONG)&clmbtSrcForBase[32];
			long				lHeight = *(LPLONG)&clmbtSrcForBase[36];
			WORD				wBpp = wBppForDiff;

			cliWork.Init(pclArc, lWidth, lHeight, wBpp);
			cliWork.WriteReverse(&clmbtDstForBase[0], dwDstSizeForBase);

			return TRUE;
		}
		else if ((lWidthForDiff >= lWidthForBase) && (lHeightForDiff >= lHeightForBase)) {
			// 差分の方が大きい

			DWORD				dwDstSizeForBase = *(LPDWORD)&clmbtSrcForBase[8];
			YCMemory<BYTE>		clmbtDstForBase(dwDstSizeForBase);

			// ベースファイルの解凍

			DecompImage(&clmbtDstForBase[0], dwDstSizeForBase, &clmbtSrcForBase[64], *(LPDWORD)&clmbtSrcForBase[16]);

			// 差分のサイズでメモリ確保

			DWORD				dwDstSize = lWidthForDiff * lHeightForDiff * (wBppForDiff >> 3);
			YCMemory<BYTE>		clmbtDst(dwDstSize);

			memset(&clmbtDst[0], 0, dwDstSize);

			// ベースファイルを右下に合わせて配置

			long				lX = lWidthForDiff - lWidthForBase;
			long				lY = lHeightForDiff - lHeightForBase;
			LPBYTE				pbyDstForBase = &clmbtDstForBase[0];
			LPBYTE				pbyDst = &clmbtDst[0];

			long				lGapForX = lX * (wBppForDiff >> 3);
			long				lLineForBase = lWidthForBase * (wBppForDiff >> 3);
			long				lLineForDiff = lWidthForDiff * (wBppForDiff >> 3);

			// 縦位置を下に合わせる
			pbyDst += lY * lLineForDiff;

			for (long y = lY; y < lHeightForDiff; y++) {
				// 横位置を右に合わせる
				pbyDst += lGapForX;

				memcpy(pbyDst, pbyDstForBase, lLineForBase);

				pbyDst += lLineForBase;
				pbyDstForBase += lLineForBase;
			}

			// ベースファイルと差分ファイルを合成

			Compose(&clmbtDst[0], dwDstSize, pbyDiff, dwDiffSize, lWidthForDiff, lWidthForDiff, wBppForDiff);

			// 出力

			CImage				cliWork;
			long				lWidth = lWidthForDiff;
			long				lHeight = lHeightForDiff;
			WORD				wBpp = wBppForDiff;

			cliWork.Init(pclArc, lWidth, lHeight, wBpp);
			cliWork.WriteReverse(&clmbtDst[0], dwDstSize);

			return TRUE;
		}
	}

	// ベースファイルが存在しない

	long				lWidth = lWidthForDiff;
	long				lHeight = lHeightForDiff;
	WORD				wBpp = wBppForDiff;

	// 真っ黒のデータを用意

	DWORD				dwDstSize = ((lWidth * (wBpp >> 3) + 3) & 0xFFFFFFFC) * lHeight;
	YCMemory<BYTE>		clmbtDst(dwDstSize);

	memset(&clmbtDst[0], 0, dwDstSize);

	// 真っ黒のデータと合成

	Compose(&clmbtDst[0], dwDstSize, pbyDiff, dwDiffSize, lWidth, lWidth, wBpp);

	CImage				cliWork;

	cliWork.Init(pclArc, lWidth, lHeight, wBpp);
	cliWork.WriteReverse(&clmbtDst[0], dwDstSize);

	// エラーメッセージの出力
/*
	if (pclArc->GetFlag()) {
		TCHAR				szError[1024];

		_stprintf(szError, _T("%s\n\n元画像が見つからなかったため、差分合成を行わずに出力しました。"), pstfiDiff->name);

		MessageBox(pclArc->GetProg()->GetHandle(), szError, _T("報告"), MB_OK);
	}
*/
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	基底画像と差分画像を合成する
//
//	十一寒月氏が作成・公開しているiarのソースコードを参考にして作成しました。

BOOL CKatakoi::Compose(LPBYTE pbyDst, DWORD dwDstSize, LPBYTE pbySrc, DWORD dwSrcSize, long lWidthForDst, long lWidthForSrc, WORD wBpp)
{
	WORD wColors = wBpp >> 3;
	DWORD dwLine = lWidthForSrc * wColors;
	DWORD dwHeight = *(LPDWORD)&pbySrc[8];

	DWORD dwGapForX = 0;

	if (lWidthForDst > lWidthForSrc) {
		dwGapForX = (lWidthForDst - lWidthForSrc) * wColors;
	}

	DWORD dwSrc = 12;
	DWORD dwDst = *(LPDWORD)&pbySrc[4] * (dwGapForX + dwLine);

	while ((dwHeight--) && (dwSrc < dwSrcSize)) {
		for (DWORD x = 0; x < dwGapForX; x++) {
			pbyDst[dwDst++] = 0;
		}

		DWORD dwCount = *(LPWORD)&pbySrc[dwSrc];
		dwSrc += 2;

		DWORD dwOffset = 0;

		while (dwCount--) {
			dwOffset += *(LPWORD)&pbySrc[dwSrc] * wColors;
			dwSrc += 2;

			DWORD dwLength = *(LPWORD)&pbySrc[dwSrc] * wColors;
			dwSrc += 2;

			while (dwLength--) {
				pbyDst[dwDst + dwOffset++] = pbySrc[dwSrc++];

				if ((dwDst + dwOffset) >= dwDstSize) {
					return TRUE;
				}
				if (dwSrc >= dwSrcSize) {
					return TRUE;
				}
			}
		}

		dwDst += dwLine;
	}

	return TRUE;
}