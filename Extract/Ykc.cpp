#include "stdafx.h"
#include "../ExtractBase.h"
#include "Ykc.h"

BOOL CYkc::Mount(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "YKC001", 6) != 0)
		return FALSE;

	// インデックスへのオフセットとインデックスのサイズの取得

	DWORD				dwIndexOffset;
	DWORD				dwIndexSize;

	pclArc->SeekHed(0x10);
	pclArc->Read(&dwIndexOffset, 4);
	pclArc->Read(&dwIndexSize, 4);

	// インデックスの取得

	YCMemory<BYTE>		clmbtIndex(dwIndexSize);

	pclArc->SeekHed(dwIndexOffset);
	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// ファイル名インデックスへのオフセットとファイル名インデックスのサイズの取得

	DWORD				dwFileNameIndexOffset;
	DWORD				dwFileNameIndexSize;

	dwFileNameIndexOffset = *(LPDWORD)&clmbtIndex[0];
	dwFileNameIndexSize = dwIndexOffset - dwFileNameIndexOffset;

	// ファイル名インデックスの取得

	YCMemory<BYTE> clmbtFileNameIndex(dwFileNameIndexSize);

	pclArc->SeekHed(dwFileNameIndexOffset);
	pclArc->Read(&clmbtFileNameIndex[0], dwFileNameIndexSize);

	// ファイル情報の取得

	for (DWORD i = 0, j = 0; i < dwIndexSize; i += 20) {
		// ファイル名の長さの取得

		DWORD				dwFileNameLen;

		dwFileNameLen = *(LPDWORD)&clmbtIndex[i + 4];

		// ファイル名の取得

		TCHAR				szFileName[_MAX_FNAME];

		lstrcpy(szFileName, (LPCTSTR)&clmbtFileNameIndex[j]);
		j += dwFileNameLen;

		// ファイル情報の取得

		SFileInfo			stfiWork;

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

	if (DecodeYKS(pclArc) == TRUE)
		return TRUE;

	if (DecodeYKG(pclArc) == TRUE)
		return TRUE;

	return FALSE;
}

BOOL CYkc::DecodeYKS(CArcFile* pclArc)
{
	SFileInfo*			pstfiWork = pclArc->GetOpenFileInfo();

	if (pstfiWork->format != _T("YKS"))
		return FALSE;

	// YKSファイルの読み込み

	YCMemory<BYTE>		clmbtSrc(pstfiWork->sizeCmp);

	pclArc->Read(&clmbtSrc[0], pstfiWork->sizeCmp);

	if (memcmp(&clmbtSrc[0], "YKS001", 6) == 0) {
		// YKS001

		// テキスト部分へのオフセットの取得

		DWORD				dwTextOffset;

		dwTextOffset = *(LPDWORD)&clmbtSrc[0x20];

		// テキスト部分の復号

		for (DWORD i = dwTextOffset; i < pstfiWork->sizeCmp; i++) {
			clmbtSrc[i] ^= 0xAA;
		}

		// 出力

		pclArc->OpenScriptFile();
		pclArc->WriteFile(&clmbtSrc[0], pstfiWork->sizeCmp);
	}
	else {
		// その他

		pclArc->OpenFile();
		pclArc->WriteFile(&clmbtSrc[0], pstfiWork->sizeCmp);
	}

	return TRUE;
}

BOOL CYkc::DecodeYKG(CArcFile* pclArc)
{
	SFileInfo*			pstfiWork = pclArc->GetOpenFileInfo();

	if (pstfiWork->format != _T("YKG"))
		return FALSE;

	// YKGファイルの読み込み

	YCMemory<BYTE>		clmbtSrc(pstfiWork->sizeCmp);

	pclArc->Read(&clmbtSrc[0], pstfiWork->sizeCmp);

	if (memcmp(&clmbtSrc[0], "YKG000", 6) == 0) {
		// YKG000

		// PNGヘッダの修正

		memcpy(&clmbtSrc[0x41], "PNG", 3);

		// 出力

		pclArc->OpenFile( _T(".png") );
		pclArc->WriteFile( &clmbtSrc[0x40], pstfiWork->sizeCmp - 0x40 );
	}
	else {
		// その他

		pclArc->OpenFile();
		pclArc->WriteFile(&clmbtSrc[0], pstfiWork->sizeCmp);
	}

	return TRUE;
}