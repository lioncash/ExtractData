#include "stdafx.h"
#include "../ExtractBase.h"
#include "Izumo.h"

// IZUMOの*.datのファイル情報を取得する関数
BOOL CIzumo::Mount(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".dat")) || (memcmp(pclArc->GetHed(), "PAK0", 4) != 0))
		return FALSE;

	// インデックスサイズ取得(最初のファイルの開始アドレス)
	DWORD index_size;
	pclArc->Seek(4, FILE_BEGIN);
	pclArc->Read(&index_size, 4);

	// スキップするバイト数取得

	DWORD				dwDummys;
	DWORD				dwSkip;

	pclArc->Read( &dwDummys, 4 );
	dwSkip = dwDummys << 3;

	// ファイル数取得
	DWORD ctFile;
	pclArc->Read(&ctFile, 4);

	// 不明なデータをスキップ
	pclArc->Seek(dwSkip, FILE_CURRENT);

	// スキップした分引く
	index_size -= 16 + dwSkip;

	// インデックス取得
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Read(pIndex, index_size);

	// ファイル名インデックス取得
	LPBYTE pFileNameIndex = pIndex + (ctFile << 4);

	// 不要なファイル名の除去

	for( DWORD i = 1 ; i < dwDummys ; i++ )
	{
		pFileNameIndex += pFileNameIndex[0] + 1;
	}

	for (int i = 0; i < (int)ctFile; i++) {
		// ファイル名取得
		TCHAR szFileName[256];
		BYTE len = *pFileNameIndex++;
		memcpy(szFileName, pFileNameIndex, len);
		szFileName[len] = _T('\0');

		// リストビューに追加
		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.sizeOrg = *(LPDWORD)&pIndex[4];
		infFile.sizeCmp = infFile.sizeOrg;
		infFile.start = *(LPDWORD)&pIndex[0];
		infFile.end = infFile.start + infFile.sizeCmp;
		pclArc->AddFileInfo(infFile);

		pIndex += 16;
		pFileNameIndex += len;
	}

	return TRUE;
}