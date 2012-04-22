#include "stdafx.h"
#include "../ExtractBase.h"
#include "MeltyBlood.h"

// MELTY BLOODのdata**.pのファイル情報を取得する関数
BOOL CMeltyBlood::Mount(CArcFile* pclArc)
{
	if ((pclArc->GetHed()[0] != 0x00) && (pclArc->GetHed()[0] != 0x01))
		return FALSE;

	if ((pclArc->GetArcName().Left(5) != _T("data0")) || (pclArc->GetArcExten() != _T(".p")))
		return FALSE;

	DWORD deckey = 0xe3df59ac;

	// ファイル数取得
	DWORD ctFile;
	pclArc->Seek(4, FILE_BEGIN);
	pclArc->Read(&ctFile, 4);
	ctFile ^= deckey;

	// ファイル数からインデックスサイズ取得
	DWORD index_size = ctFile * 68;

	// インデックス取得
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Read(pIndex, index_size);

	for (int i = 0; i < (int)ctFile; i++) {
		// ファイル名取得
		TCHAR szFileName[60];
		memcpy(szFileName, pIndex, 60);
		// ファイル名復号
		for (int j = 0; j < 59; j++)
			szFileName[j] ^= i * j * 3 + 61;

		// リストビューに追加
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

	return TRUE;
}

// 抽出する関数
BOOL CMeltyBlood::Decode(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->format != _T("MeltyBlood"))
		return FALSE;

	// 出力ファイル生成
	pclArc->OpenFile();

	// 0x00～0x2173までのデータを復号し、出力
	Decrypt(pclArc);

	// 0x2173より先をそのまま出力
	if (pInfFile->sizeOrg > 0x2173)
		pclArc->ReadWrite(pInfFile->sizeOrg - 0x2173);

	return TRUE;
}

// データを復号する関数
void CMeltyBlood::Decrypt(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	// バッファ確保
	int lim = (pInfFile->sizeOrg < 0x2173) ? pInfFile->sizeOrg : 0x2173;
	YCMemory<BYTE> buf(lim);

	pclArc->Read(&buf[0], lim);
	// 復号
	int keylen = (int)pInfFile->name.GetLength();
	for (int i = 0; i < lim; i++)
		buf[i] ^= pInfFile->name[i % keylen] + i + 3;
	pclArc->WriteFile(&buf[0], lim);
}