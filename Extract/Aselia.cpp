#include "stdafx.h"
#include "../ExtractBase.h"
#include "Aselia.h"

BOOL CAselia::Mount(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".gd"))
		return FALSE;

	TCHAR szDllPath[MAX_PATH];
	lstrcpy(szDllPath, pclArc->GetArcPath());
	PathRenameExtension(szDllPath, _T(".dll"));
	if (PathFileExists(szDllPath) == FALSE)
		return FALSE;

	// DLLファイルを開く
	CFile DllFile;
	if (DllFile.Open(szDllPath, FILE_READ) == INVALID_HANDLE_VALUE)
		return FALSE;

	// ファイル数取得
	DWORD ctFile;
	DllFile.Read(&ctFile, 4);
	ctFile--;

	// ファイル数からインデックスサイズ取得
	DWORD index_size = ctFile << 3;

	// インデックス取得
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	DllFile.Read(pIndex, index_size);

	// 拡張子取得
	YCString sFileExt;
	if (memcmp(&pclArc->GetHed()[4], "OggS", 4) == 0)
		sFileExt = _T(".ogg");
	else if (memcmp(&pclArc->GetHed()[4], "臼NG", 4) == 0)
		sFileExt = _T(".png");

	for (int i = 0; i < (int)ctFile; i++) {
		// ファイル名取得
		TCHAR szFileName[MAX_PATH];
		_stprintf(szFileName, _T("%s_%06d%s"), pclArc->GetArcName(), i + 1, sFileExt);

		SFileInfo infFile;
		infFile.start = *(LPDWORD)&pIndex[0];
		infFile.sizeOrg = *(LPDWORD)&pIndex[4];

		// リストビューに追加
		infFile.name = szFileName;
		infFile.sizeCmp = infFile.sizeOrg;
		infFile.end = infFile.start + infFile.sizeOrg;
		pclArc->AddFileInfo(infFile);

		pIndex += 8;
	}

	return TRUE;
}