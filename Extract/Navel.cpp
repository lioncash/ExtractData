#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Wav.h"
#include "Navel.h"

// Navelのpacファイル情報を取得する関数
BOOL CNavel::Mount(CArcFile* pclArc)
{
	if (MountPac(pclArc) == TRUE)
		return TRUE;
	if (MountWpd(pclArc) == TRUE)
		return TRUE;
	return FALSE;
}

BOOL CNavel::MountPac(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".pac")) || (memcmp(pclArc->GetHed(), "CAPF", 4) != 0))
		return FALSE;

	// インデックスサイズ取得
	DWORD index_size;
	pclArc->Seek(8, FILE_BEGIN);
	pclArc->Read(&index_size, 4);
	index_size -= 32;

	// ファイル数取得
	DWORD ctFile;
	pclArc->Read(&ctFile, 4);

	// インデックス取得
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Seek(16, FILE_CURRENT);
	pclArc->Read(pIndex, index_size);

	for (int i = 0; i < (int)ctFile; i++) {
		// ファイル名取得
		TCHAR szFileName[32];
		memcpy(szFileName, &pIndex[8], 32);

		// リストビューに追加
		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.sizeOrg = *(LPDWORD)&pIndex[4];
		infFile.sizeCmp = infFile.sizeOrg;
		infFile.start = *(LPDWORD)&pIndex[0];
		infFile.end = infFile.start + infFile.sizeOrg;
		pclArc->AddFileInfo(infFile);

		pIndex += 40;
	}

	return TRUE;
}

BOOL CNavel::MountWpd(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".WPD")) || (memcmp(pclArc->GetHed(), " DPW", 4) != 0))
		return FALSE;
	return pclArc->Mount();
}

// WAVに変換して抽出する関数
BOOL CNavel::Decode(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->format != _T("WPD"))
		return FALSE;

	// WPDフォーマット読み込み
	FormatWPD fWPD;
	pclArc->Read(&fWPD, sizeof(FormatWPD));

	// 出力
	CWav wav;
	wav.Init(pclArc, pInfFile->sizeOrg - 44, fWPD.freq, fWPD.channels, fWPD.bits);
	wav.Write();

	return TRUE;
}