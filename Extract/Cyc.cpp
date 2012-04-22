#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Image.h"
//#include "../Image/Jpg.h"
#include "Cyc.h"

///////////////////////////////////////////////////////////////////////////
//	マウント
///////////////////////////////////////////////////////////////////////////
BOOL CCyc::Mount(CArcFile* pclArc)
{
	if (MountGpk(pclArc) == TRUE)
		return TRUE;
	if (MountVpk(pclArc) == TRUE)
		return TRUE;
	if (MountDwq(pclArc) == TRUE)
		return TRUE;
	if (MountWgq(pclArc) == TRUE)
		return TRUE;
	if (MountVaw(pclArc) == TRUE)
		return TRUE;
	if (MountXtx(pclArc) == TRUE)
		return TRUE;
	if (MountFxf(pclArc) == TRUE)
		return TRUE;

	return FALSE;
}

BOOL CCyc::MountGpk(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".gpk"))
		return FALSE;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return FALSE;

	// gtbファイルへのパス取得
	TCHAR szGtbPath[MAX_PATH];
	lstrcpy(szGtbPath, pclArc->GetArcPath());
	PathRenameExtension(szGtbPath, _T(".gtb"));

	// gtbファイルを開く
	CFile GtbFile;
	if (GtbFile.Open(szGtbPath, FILE_READ) == INVALID_HANDLE_VALUE)
		return FALSE;

	// gtbファイルからファイル数取得
	DWORD ctFile;
	GtbFile.Read(&ctFile, 4);

	// テーブルサイズを求める
	DWORD tblSize = ctFile * 4;

	// gtbファイルからファイル名長テーブル取得
	YCMemory<DWORD> fnLenTbl(ctFile);
	GtbFile.Read(&fnLenTbl[0], tblSize);

	// gtbファイルから開始オフセットテーブル取得
	YCMemory<DWORD> ofsTbl(ctFile);
	GtbFile.Read(&ofsTbl[0], tblSize);

	// ファイル名テーブルサイズを求める
	DWORD fnTblSize = GtbFile.GetFileSize() - 4 - tblSize * 2;

	// gtbファイルからファイル名テーブル取得
	YCMemory<char> fnTbl(fnTblSize);
	GtbFile.Read(&fnTbl[0], fnTblSize);

	LPSTR pfnTbl = &fnTbl[0];

	for (int i = 0; i < (int)ctFile; i++) {
		SFileInfo infFile;
		infFile.name = pfnTbl;
		infFile.name += _T(".dwq"); // gpkなのでdwqがアーカイブされている
		infFile.start = ofsTbl[i];
		infFile.end = (i + 1 == ctFile) ? pclArc->GetArcSize() : ofsTbl[i+1]; // 一番最後のファイルの終了位置はgpkファイルの終端
		infFile.sizeCmp = infFile.end - infFile.start;
		infFile.sizeOrg = infFile.sizeCmp;
		pclArc->AddFileInfo(infFile);

		pfnTbl += infFile.name.GetLength() - 3;
	}

	return TRUE;
}

BOOL CCyc::MountVpk(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".vpk"))
		return FALSE;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return FALSE;

	// vtbファイルへのパス取得
	TCHAR szVtbPath[MAX_PATH];
	lstrcpy(szVtbPath, pclArc->GetArcPath());
	PathRenameExtension(szVtbPath, _T(".vtb"));

	// vtbファイルを開く
	CFile VtbFile;
	if (VtbFile.Open(szVtbPath, FILE_READ) == INVALID_HANDLE_VALUE)
		return FALSE;

	// vtbファイル読み込み
	DWORD vtbSize = VtbFile.GetFileSize();
	YCMemory<BYTE> vtb(vtbSize);
	VtbFile.Read(&vtb[0], vtbSize);

	LPBYTE pvtb = &vtb[0];

	while (1) {
		char				szFileTitle[12];

		memcpy( szFileTitle, pvtb, 8 );
		szFileTitle[8] = '\0';

		SFileInfo infFile;
		infFile.name = szFileTitle;
		infFile.name += _T(".vaw");
		infFile.start = *(LPDWORD)&pvtb[8];
		infFile.end = *(LPDWORD)&pvtb[20];
		infFile.sizeCmp = infFile.end - infFile.start;
		infFile.sizeOrg = infFile.sizeCmp;
		pclArc->AddFileInfo(infFile);

		pvtb += 12;
		if (pvtb[0] == _T('\0'))
			break;
	}

	return TRUE;
}

BOOL CCyc::MountDwq(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".dwq"))
		return FALSE;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return FALSE;

	return pclArc->Mount();
}

BOOL CCyc::MountWgq(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".wgq"))
		return FALSE;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return FALSE;

	return pclArc->Mount();
}

BOOL CCyc::MountVaw(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".vaw"))
		return FALSE;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return FALSE;

	return pclArc->Mount();
}

BOOL CCyc::MountXtx(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".xtx"))
		return FALSE;

	return pclArc->Mount();
}

BOOL CCyc::MountFxf(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".fxf"))
		return FALSE;

	return pclArc->Mount();
}


///////////////////////////////////////////////////////////////////////////
//	デコード
///////////////////////////////////////////////////////////////////////////
BOOL CCyc::Decode(CArcFile* pclArc)
{
	if (DecodeDwq(pclArc) == TRUE)
		return TRUE;
	if (DecodeWgq(pclArc) == TRUE)
		return TRUE;
	if (DecodeVaw(pclArc) == TRUE)
		return TRUE;
	if (DecodeXtx(pclArc) == TRUE)
		return TRUE;
	if (DecodeFxf(pclArc) == TRUE)
		return TRUE;

	return FALSE;
}

BOOL CCyc::DecodeDwq(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->format != _T("DWQ"))
		return FALSE;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return FALSE;

	// 読み込み
	YCMemory<BYTE> buf(pInfFile->sizeCmp);
	pclArc->Read(&buf[0], pInfFile->sizeCmp);

	// JPEG + MASK(PACKBMP)
	if (memcmp(&buf[0], "JPEG+MASK", 9) == 0) {
		pclArc->OpenFile(_T(".jpg"));
		pclArc->WriteFile(&buf[64], *(LPDWORD)&buf[32]);

		//CJpg jpg;
		//DWORD dstSize;
		//LPBYTE dst = jpg.Decomp(&buf[64], *(LPDWORD)&buf[32], &dstSize);

		//DWORD maskOfs = 64 + *(LPDWORD)&buf[32];

		//// BMPヘッダ取得
		//LPBITMAPFILEHEADER fHed = (LPBITMAPFILEHEADER)&buf[maskOfs];
		//LPBITMAPINFOHEADER iHed = (LPBITMAPINFOHEADER)&buf[maskOfs+14];

		//// 出力用バッファ確保
		//DWORD maskSize = fHed->bfSize - fHed->bfOffBits;
		//YCMemory<BYTE> mask(maskSize);

		//// RLE解凍
		//DecompRLE(&mask[0], &buf[maskOfs+fHed->bfOffBits], iHed->biWidth, iHed->biHeight);

		//// アルファブレンド
		//jpg.AlphaBlend(dst, &mask[0], 0x00);

		//// 出力
		//CImage image;
		//image.Init(pclArc, jpg.GetWidth(), jpg.GetHeight(), 24);
		//image.WriteReverse(dst, dstSize);
	}
	// JPEG
	else if (memcmp(&buf[0], "JPEG", 4) == 0) {
		pclArc->OpenFile(_T(".jpg"));
		pclArc->WriteFile(&buf[64], *(LPDWORD)&buf[32]);
	}
	// BMP(RGB)
	else if (memcmp(&buf[0], "BMP", 3) == 0) {
		// BMPヘッダ取得
		LPBITMAPFILEHEADER fHed = (LPBITMAPFILEHEADER)&buf[64];
		LPBITMAPINFOHEADER iHed = (LPBITMAPINFOHEADER)&buf[78];

		// RGBと並んでいるのをBGRに並べ替える
		if (iHed->biBitCount == 24)
			RGBtoBGR(&buf[64+54], iHed->biWidth, iHed->biHeight);

		// サイズのずれを調整
		DWORD dstSize = fHed->bfSize - fHed->bfOffBits;
		if (((iHed->biWidth * (iHed->biBitCount >> 3) + 3) & 0xFFFFFFFC) * iHed->biHeight != iHed->biSizeImage)
			dstSize -= 2;

		// 出力
		CImage image;
		image.Init(pclArc, iHed->biWidth, iHed->biHeight, iHed->biBitCount, &buf[64+54], fHed->bfOffBits - 54);
		image.WriteReverse(&buf[64 + fHed->bfOffBits], dstSize);
	}
	// PACKBMP
	else if (memcmp(&buf[0], "PACKBMP", 7) == 0) {
		// BMPヘッダ取得
		LPBITMAPFILEHEADER fHed = (LPBITMAPFILEHEADER)&buf[64];
		LPBITMAPINFOHEADER iHed = (LPBITMAPINFOHEADER)&buf[78];

		// 出力用バッファ確保
		DWORD dstSize = fHed->bfSize - fHed->bfOffBits;
		YCMemory<BYTE> dst(dstSize);

		// RLE解凍
		DecompRLE(&dst[0], &buf[64+fHed->bfOffBits], iHed->biWidth, iHed->biHeight);

		// 出力
		CImage image;
		image.Init(pclArc, iHed->biWidth, iHed->biHeight, iHed->biBitCount, &buf[64+54], fHed->bfOffBits - 54);
		image.WriteReverse(&dst[0], dstSize - 1); // 強制的にダミーデータ出力
	}
	// BMP
	else if (memcmp(&buf[64], "BM", 2) == 0) {
		// BMPヘッダ取得
		LPBITMAPFILEHEADER fHed = (LPBITMAPFILEHEADER)&buf[64];
		LPBITMAPINFOHEADER iHed = (LPBITMAPINFOHEADER)&buf[78];

		if (memcmp(&buf[48], "PACKTYPE=1", 10) == 0) {
			// 出力用バッファ確保
			DWORD dstSize = fHed->bfSize - fHed->bfOffBits;
			YCMemory<BYTE> dst(dstSize);

			// RLE解凍
			DecompRLE(&dst[0], &buf[64+fHed->bfOffBits], iHed->biWidth, iHed->biHeight);

			// 出力
			CImage image;
			image.Init(pclArc, iHed->biWidth, iHed->biHeight, iHed->biBitCount, &buf[64+54], fHed->bfOffBits - 54);
			image.WriteReverse(&dst[0], dstSize - 1); // 強制的にダミーデータ出力
		}
		else {
			// 出力
			CImage image;
			image.Init(pclArc, iHed->biWidth, iHed->biHeight, iHed->biBitCount, &buf[64+54], fHed->bfOffBits - 54);
			image.WriteReverse(&buf[64 + fHed->bfOffBits], fHed->bfSize - fHed->bfOffBits);
		}
	}
	// その他
	else {
		pclArc->OpenFile(_T(".dwq"));
		pclArc->WriteFile(&buf[0], pInfFile->sizeCmp);
	}

	return TRUE;
}

BOOL CCyc::DecodeWgq(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->format != _T("WGQ"))
		return FALSE;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return FALSE;

	// 読み込み
	YCMemory<BYTE> buf(pInfFile->sizeCmp);
	pclArc->Read(&buf[0], pInfFile->sizeCmp);

	// 出力
	pclArc->OpenFile(_T(".ogg"));
	pclArc->WriteFile(&buf[64], pInfFile->sizeCmp);

	return TRUE;
}

BOOL CCyc::DecodeVaw(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->format != _T("VAW"))
		return FALSE;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return FALSE;

	// 読み込み
	YCMemory<BYTE> buf(pInfFile->sizeCmp);
	pclArc->Read(&buf[0], pInfFile->sizeCmp);

	// ogg
	if (memcmp(&buf[108], "OggS", 4) == 0) {
		pclArc->OpenFile(_T(".ogg"));
		pclArc->WriteFile(&buf[108], pInfFile->sizeCmp - 108);
	}
	// wav
	else {
		pclArc->OpenFile(_T(".wav"));
		pclArc->WriteFile(&buf[64], pInfFile->sizeCmp - 64);
	}

	return TRUE;
}

BOOL CCyc::DecodeXtx(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->format != _T("XTX"))
		return FALSE;

	// 読み込み
	YCMemory<BYTE> buf(pInfFile->sizeCmp);
	pclArc->Read(&buf[0], pInfFile->sizeCmp);

	// 出力
	pclArc->OpenScriptFile();
	pclArc->WriteFile(&buf[0], pInfFile->sizeCmp);

	return TRUE;
}

BOOL CCyc::DecodeFxf(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->format != _T("FXF"))
		return FALSE;

	// 読み込み
	YCMemory<BYTE> buf(pInfFile->sizeCmp);
	pclArc->Read(&buf[0], pInfFile->sizeCmp);

	// 復号
	for (int i = 0; i < (int)pInfFile->sizeCmp; i++)
		buf[i] ^= 0xFF;

	// 出力
	pclArc->OpenScriptFile();
	pclArc->WriteFile(&buf[0], pInfFile->sizeCmp);

	return TRUE;
}

void CCyc::DecompRLE(LPBYTE dst, LPBYTE src, LONG width, LONG height)
{
	// 1行バッファ確保
	YCMemory<BYTE> dataWidth(width);
	YCMemory<BYTE> predataWidth(width);
	memset(&predataWidth[0], 0, width);

	for (int i = 0; i < height; i++) {
		// 1行バッファのポインタ
		LPBYTE pdataWidth = &dataWidth[0];

		// 1行ずつ処理していく
		for (int j = 0; j < width; ) {
			// 入力データを順次取得
			BYTE data = *src++;
			// 入力データが0ならばRLEなので解凍
			if (data == 0) {
				// 次の入力データには長さが書いてある
				BYTE len = *src++;
				// RLEを解凍
				for (int k = 0; k < (int)len; k++)
					*pdataWidth++ = data;
				// 解凍した長さ進める
				j += len;
			}
			// それ以外は無圧縮データなのでそのまま辞書に入力
			else {
				*pdataWidth++ = data;
				j++;
			}
		}

		// 今回の行のデータを1行前のデータを使って復号
		for (int j = 0; j < width; j++) {
			dataWidth[j] ^= predataWidth[j];
			// 次の行の復号時に使うため、今回の行のデータを保存
			predataWidth[j] = dataWidth[j];
		}

		// 復号したデータを出力バッファに出力
		pdataWidth = &dataWidth[0];
		for (int j = 0; j < width; j++)
			*dst++ = *pdataWidth++;
	}
}

void CCyc::RGBtoBGR(LPBYTE buf, LONG width, LONG height)
{
	// 幅を求める
	LONG lwidth = width * 3;
	// パディング付きの幅を求める
	LONG lwidth_rev = (lwidth + 3) & 0xFFFFFFFC;

	for (int y = 0; y < height; y++) {
		// RとBを入れ替える
		for (int x = 0; x < lwidth; x += 3) {
			BYTE tmp = buf[0];
			buf[0] = buf[2];
			buf[2] = tmp;
			buf += 3;
		}
		// パディングはスキップ
		for (int x = lwidth; x < lwidth_rev; x++)
			buf++;
	}
}