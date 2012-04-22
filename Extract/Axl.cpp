#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Arc/LZSS.h"
#include "../Image.h"
#include "Axl.h"

BOOL CAxl::Mount(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".arc"))
		return FALSE;
	if (*(LPDWORD)&pclArc->GetHed()[4] != 0 && *(LPDWORD)&pclArc->GetHed()[4] != 1)
		return FALSE;

	// ファイル数取得
	DWORD ctFile = *(LPDWORD)&pclArc->GetHed()[0];

	// ファイルタイプ取得
	DWORD type = *(LPDWORD)&pclArc->GetHed()[4];

	// ファイル数からインデックスサイズ取得
	DWORD index_size = ctFile * 44;

	// アーカイブファイルサイズがインデックスサイズより小さかったら無関係のアーカイブファイルと判定
	if (pclArc->GetArcSize() < index_size)
		return FALSE;

	pclArc->Seek(8, FILE_BEGIN);

	// インデックス取得
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Read(pIndex, index_size);

	// インデックス復号
	if (DecryptIndex(pIndex, index_size, pclArc->GetArcSize()) == FALSE) {
		// 復号できなかったら無関係のアーカイブファイルと判定
		pclArc->SeekHed();
		return FALSE;
	}

	for (int i = 0; i < (int)ctFile; i++) {
		// ファイル名取得
		TCHAR szFileName[33];
		memcpy(szFileName, pIndex, 32);
		szFileName[32] = _T('\0');

		// リストビューに追加
		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.sizeOrg = *(LPDWORD)&pIndex[32];
		infFile.sizeCmp = *(LPDWORD)&pIndex[36];
		infFile.start = *(LPDWORD)&pIndex[40];
		infFile.end = infFile.start + infFile.sizeCmp;
		if (type == 1) infFile.format = _T("LZ");
		infFile.title = _T("AXL");
		pclArc->AddFileInfo(infFile);

		pIndex += 44;
	}

	return TRUE;
}

BOOL CAxl::Decode(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->title != _T("AXL"))
		return FALSE;

	// バッファ確保
	YCMemory<BYTE> buf(pInfFile->sizeOrg);

	if (pInfFile->format == _T("LZ")) {
		// LZ圧縮ファイル

		// 読み込み
		YCMemory<BYTE> z_buf(pInfFile->sizeCmp);
		pclArc->Read(&z_buf[0], pInfFile->sizeCmp);

		// LZSS解凍

		CLZSS				clLZSS;

		clLZSS.Decomp( &buf[0], pInfFile->sizeOrg, &z_buf[0], pInfFile->sizeCmp, 4096, 4078, 3 );
	}
	else {
		// 無圧縮ファイル

		// 読み込み
		pclArc->Read(&buf[0], pInfFile->sizeCmp);
	}

	if (lstrcmpi(PathFindExtension(pInfFile->name), _T(".bmp")) == 0) {
		// BMPファイル

		LPBITMAPFILEHEADER fHed = (LPBITMAPFILEHEADER)&buf[0];
		LPBITMAPINFOHEADER iHed = (LPBITMAPINFOHEADER)&buf[14];

		if (fHed->bfSize != pInfFile->sizeOrg) {
			// 32bitのBMP

			// 32bitのBMPを構築するバッファ
			YCMemory<BYTE> buf2(pInfFile->sizeOrg - 54);

			// α値を指す
			LPBYTE pbufA = &buf[fHed->bfSize];

			// BMPヘッダを飛ばす
			LPBYTE pbuf = &buf[54];

			LPBYTE pbuf2 = &buf2[0];

			DWORD color_size = iHed->biWidth * iHed->biHeight;

			for (int i = 0; i < (int)color_size; i++) {
				// α値合成
				for (int j = 0; j < 3; j++)
					*pbuf2++ = *pbuf++;
				*pbuf2++ = *pbufA++;
			}

			CImage image;
			image.Init(pclArc, iHed->biWidth, iHed->biHeight, 32);
			image.Write(&buf2[0], pInfFile->sizeOrg - 54);
		}
		else {
			// 24bit以下のBMP

			CImage image;
			image.Init(pclArc, &buf[0]);
			image.Write(pInfFile->sizeOrg);
		}
	}
	else if (pInfFile->format == _T("LZ")) {
		// BMP以外のLZ圧縮ファイル

		pclArc->OpenFile();
		pclArc->WriteFile(&buf[0], pInfFile->sizeOrg);
	}
	else {
		// その他のファイル

		pclArc->OpenFile();
		pclArc->WriteFile(&buf[0], pInfFile->sizeCmp);
	}

	return TRUE;
}

void CAxl::InitMountKey(LPVOID deckey)
{
	m_len = strlen((char*)deckey);
	memcpy(m_deckey, deckey, m_len);
}

BOOL CAxl::CreateKey(LPBYTE key, LPINT key_len, LPBYTE pIndex, DWORD index_size)
{
	for (int i = 0; i < (int)index_size; i += 44) {
		// インデックスからファイル名の部分をコピー
		BYTE fname[32];
		memcpy(fname, pIndex, 32);

		LPBYTE key_end = &fname[31];

		LPBYTE pkey1 = key_end;
		LPBYTE pkey2 = key_end - 1;

		*key_len = 1;

		while (pkey2 > fname) {
			while (*pkey1 != *pkey2) {
				// 同じデータが出るまで前に移動
				pkey2--;

				// keyの長さを数えておく
				(*key_len)++;

				// ファイル名の先頭まで戻ったらループを抜ける
				if (pkey2 == fname)
					break;
			}

			// ファイル名の先頭まで戻ったらループを抜ける
			if (pkey2 == fname)
				break;

			int key_len_cpy = *key_len;

			// 前のデータとの比較を行う
			while (*pkey1 == *pkey2) {
				pkey1--;
				pkey2--;
				key_len_cpy--;

				// 完全一致したらキー生成
				if (key_len_cpy == 0) {
					// 最初にコピーするキーの長さ
					int key_len_hed = (i + 32) % *key_len;
					LPBYTE pkey3 = key_end + 1 - key_len_hed;

					for (int j = 0; j < key_len_hed; j++)
						key[j] = *pkey3++;

					// 続きのキー
					pkey3 = key_end + 1 - *key_len;

					for (int j = key_len_hed; j < *key_len; j++)
						key[j] = *pkey3++;

					return TRUE;
				}

				// ファイル名の先頭まで戻ったらループを抜ける
				if (pkey2 == fname)
					break;
			}

			// ファイル名の先頭まで戻ったらループを抜ける
			if (pkey2 == fname)
				break;
		}

		pIndex += 44;
	}

	return FALSE;
}

BOOL CAxl::DecryptIndex(LPBYTE pIndex, DWORD index_size, QWORD arcSize)
{
	// インデックスのファイル名部分からキー生成
	BYTE key[32];
	int key_len;
	if (CreateKey(key, &key_len, pIndex, index_size) == FALSE)
		return FALSE;

	// インデックスをコピー
	YCMemory<BYTE> pIndex_copy(index_size);
	memcpy(&pIndex_copy[0], pIndex, index_size);

	// コピーしたインデックスを復号
	for (int i = 0, j = 0; i < (int)index_size; i++) {
		pIndex_copy[i] -= key[j++];
		if (j == key_len) j = 0;
	}

	// 最初のファイルの始点とインデックスの終点が一致するかチェック
	if (*(LPDWORD)&pIndex_copy[40] != 8 + index_size)
		return FALSE;

	// 最終ファイルの終点とアーカイブファイルの終点が一致するかチェック
	if (*(LPDWORD)&pIndex_copy[index_size-4] + *(LPDWORD)&pIndex_copy[index_size-8] != arcSize)
		return FALSE;

	// チェックの結果が問題なければ復号したインデックスをコピー
	memcpy(pIndex, &pIndex_copy[0], index_size);

	return TRUE;

/*
	static char* key[] = {
		"SUMMER",		// チュートリアルサマー
		"HIDAMARI",		// ひだまり
		"KIMIKOE",		// キミの声がきこえる
		"CFTRIAL",		// CROSS FIRE体験版v1
		"TEST",			// CROSS FIRE, CROSS FIRE体験版v2
		"SAKURA-R",		// さくらリラクゼーション
		"KOICHU",		// こいちゅ！ ～恋に恋するかたおもい～
		"LAVENDURA",	// 黎明のラヴェンデュラ
		"HAMA",			// 真章 幻夢館, そらのいろ、みずのいろ
		"OBA",			// 叔母の寝室
		"KANSEN"		// 姦染 ～淫欲の連鎖～
	};

	YCMemory<BYTE> pIndex_copy(index_size);

	for (int i = 0; i < _countof(key); i++) {
		// インデックスをコピー
		memcpy(&pIndex_copy[0], pIndex, index_size);
		// keyの長さを求めておく
		int key_len = strlen((char*)key[i]);
		// 復号
		for (int j = 0, k = 0; j < (int)index_size; j++) {
			pIndex_copy[j] += (BYTE)key[i][k++];
			if (k == key_len)
				k = 0;
		}
		// 第一ファイルの始点とインデックスの終点が一致するかチェック
		// 一致しなかったら別のkeyで復号する
		if (*(LPDWORD)&pIndex_copy[40] != 8 + index_size)
			continue;
		// 最終ファイルの終点とアーカイブファイルの終点が一致するかチェック
		// これも一致したら対応していると判定
		if (*(LPDWORD)&pIndex_copy[index_size-4] + *(LPDWORD)&pIndex_copy[index_size-8] == arcSize) {
			memcpy(pIndex, &pIndex_copy[0], index_size);
			return TRUE;
		}
	}

	return FALSE;
*/


	//LPBYTE deckey = m_deckey;
	//DWORD deckey_len = m_len;

	//for (int i = 0, j = 0; i < (int)index_size; i++) {
	//	pIndex[i] += deckey[j++];
	//	if (j == deckey_len) j = 0;
	//}
}