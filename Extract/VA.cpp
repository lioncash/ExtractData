#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Wav.h"
#include "VA.h"

BOOL CVA::Mount(CArcFile* pclArc)
{
	if (MountNwa(pclArc) == TRUE)
		return TRUE;
	if (MountNwk(pclArc) == TRUE)
		return TRUE;
	if (MountOvk(pclArc) == TRUE)
		return TRUE;
	return FALSE;
}

// *.nwaのファイル情報を取得する関数
BOOL CVA::MountNwa(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".nwa"))
		return FALSE;

	// nwaヘッダ読み込み
	NWAHed nwaHed;
	pclArc->Read(&nwaHed, sizeof(NWAHed));

	// リストビューに追加
	SFileInfo infFile;
	infFile.name = pclArc->GetArcName();
	infFile.sizeOrg = nwaHed.DataSize + 44;
	infFile.sizeCmp = (nwaHed.CompFileSize == 0) ? infFile.sizeOrg : nwaHed.CompFileSize;
	infFile.start = 0x00;
	infFile.end = infFile.start + infFile.sizeCmp;
	pclArc->AddFileInfo(infFile);

	return TRUE;
}

// *.nwkのファイル情報を取得する関数
BOOL CVA::MountNwk(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".nwk"))
		return FALSE;

	// ファイル数取得
	DWORD ctFile;
	pclArc->Read(&ctFile, 4);

	// ファイル数からインデックスサイズ取得
	DWORD index_size = ctFile * 12;

	// インデックス取得
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Read(pIndex, index_size);

	// ベースファイル名取得
	TCHAR szBaseFileName[_MAX_FNAME];
	lstrcpy(szBaseFileName, pclArc->GetArcName());
	PathRemoveExtension(szBaseFileName);

	for (int i = 0; i < (int)ctFile; i++) {
		// ファイル名取得
		TCHAR szFileName[_MAX_FNAME];
		_stprintf(szFileName, _T("%s_%06d.nwa"), szBaseFileName, *(LPDWORD) &pIndex[8]);

		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.sizeCmp = *(LPDWORD)&pIndex[0];
		//infFile.sizeOrg = infFile.sizeCmp;
		infFile.start = *(LPDWORD)&pIndex[4];
		infFile.end = infFile.start + infFile.sizeCmp;

		// ファイルサイズ取得
		pclArc->Seek(infFile.start + 20, FILE_BEGIN);
		pclArc->Read(&infFile.sizeOrg, 4);
		infFile.sizeOrg += 44;

		pclArc->AddFileInfo(infFile);

		pIndex += 12;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ovkのファイル情報を取得する

BOOL CVA::MountOvk(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".ovk"))
		return FALSE;

	// ファイル数取得

	DWORD				dwFiles;

	pclArc->Read(&dwFiles, 4);

	// インデックスサイズの取得

	DWORD				dwIndexSize;

	dwIndexSize = dwFiles * 16;

	// インデックスの読み込み

	YCMemory<BYTE>		clmbtIndex(dwIndexSize);

	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// ベースファイル名の取得

	TCHAR szBaseFileName[_MAX_FNAME];

	lstrcpy(szBaseFileName, pclArc->GetArcName());
	PathRemoveExtension(szBaseFileName);

	// ファイル情報の取得

	for (DWORD i = 0; i < dwIndexSize; i += 16) {
		// ファイル名の取得

		TCHAR				szFileName[_MAX_FNAME];

		_stprintf(szFileName, _T("%s_%06d.ogg"), szBaseFileName, *(LPDWORD) &clmbtIndex[i + 8]);

		// ファイル情報の取得

		SFileInfo			stfiWork;

		stfiWork.name = szFileName;
		stfiWork.sizeCmp = *(LPDWORD)&clmbtIndex[i];
		stfiWork.sizeOrg = stfiWork.sizeCmp;
		stfiWork.start = *(LPDWORD)&clmbtIndex[i + 4];
		stfiWork.end = stfiWork.start + stfiWork.sizeCmp;

		pclArc->AddFileInfo(stfiWork);
	}

	return TRUE;
}

inline int CVA::getbits(LPBYTE& data, int& shift, int bits)
{
	if (shift > 8) {
		data++;
		shift -= 8;
	}
	int ret = *(LPWORD)data >> shift;
	shift += bits;
	return ret & ((1 << bits) - 1); // mask
}

BOOL CVA::Decode(CArcFile* pclArc)
{
	if (DecodeNwa(pclArc) == TRUE)
		return TRUE;
	return FALSE;
}

// WAVに変換して抽出する関数
BOOL CVA::DecodeNwa(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->format != _T("NWA"))
		return FALSE;

	// nwaヘッダ読み込み
	NWAHed nwaHed;
	pclArc->Read(&nwaHed, sizeof(NWAHed));

	CWav wav;
	wav.Init(pclArc, nwaHed.DataSize, nwaHed.freq, nwaHed.channels, nwaHed.bits);

	// 無圧縮の場合、WAVヘッダに変えるだけ
	if (nwaHed.CmpLevel == 0xFFFFFFFF)
		wav.Write();
	else {
		// RLEの有効性

		BOOL				bRLE = FALSE;

		if( (nwaHed.CmpLevel == 5) && (nwaHed.channels != 2) )
		{
			// RLE有効

			bRLE = TRUE;
		}

		// オフセット用のメモリ確保
		DWORD offset_size = nwaHed.blocks * 4;
		YCMemory<DWORD> offsets(offset_size);
		// データ書き込み用のメモリ確保
		DWORD buf_len = nwaHed.BlockSize * (nwaHed.bits >> 3);
		YCMemory<BYTE> buf(buf_len);
		// データ読み込み用のメモリ確保
		DWORD z_buf_len = buf_len * 2;
		YCMemory<BYTE> z_buf(z_buf_len);

		// オフセット読み込み
		pclArc->Read(&offsets[0], offset_size);

		for (int i = 0; i < (int)nwaHed.blocks; i++) {
			LPBYTE z_pbuf = &z_buf[0];
			LPBYTE pbuf = &buf[0];

			// 今回読み込む/デコードするデータの大きさを得る
			DWORD curblocksize, curcompsize;
			if (i < (int)nwaHed.blocks - 1) {
				curblocksize = buf_len;
				curcompsize = offsets[i + 1] - offsets[i];
			}
			else {
				curblocksize = nwaHed.RestSize * (nwaHed.bits >> 3);
				curcompsize = z_buf_len;
			}

			// バッファの終端

			BYTE*				pbtSrcEnd = z_pbuf + curcompsize;
			BYTE*				pbtDstEnd = pbuf + curblocksize;

			// データ読み込み
			pclArc->Read(z_pbuf, curcompsize);

			int d[2];
			int shift = 0;

			// 最初のデータを読み込む
			if (nwaHed.bits == 8)
				d[0] = *z_pbuf++;
			else { // fNWA.bits == 16
				d[0] = *(LPWORD)z_pbuf;
				z_pbuf += 2;
			}

			if (nwaHed.channels == 2) {
				if (nwaHed.bits == 8)
					d[1] = *z_pbuf++;
				else { // fNWA.bits == 16
					d[1] = *(LPWORD)z_pbuf;
					z_pbuf += 2;
				}
			}

			int dsize = curblocksize / (nwaHed.bits >> 3);
			BOOL bFlip = 0; // ステレオ用

			for (int j = 0; j < dsize; j++)
			{
				if( (z_pbuf >= pbtSrcEnd) || (pbuf >= pbtDstEnd) )
				{
					// 終了

					break;
				}

				int type = getbits(z_pbuf, shift, 3);

				// 7 : 大きな差分
				if (type == 7) {
					if (getbits(z_pbuf, shift, 1) == 1)
						d[bFlip] = 0; // 未使用
					else {
						int BITS, SHIFT;
						if (nwaHed.CmpLevel >= 3) {
							BITS = 8;
							SHIFT = 9;
						}
						else {
							BITS = 8 - nwaHed.CmpLevel;
							SHIFT = 2 + type + nwaHed.CmpLevel;
						}
						const int MASK1 = (1 << (BITS - 1));
						const int MASK2 = (1 << (BITS - 1)) - 1;
						int b = getbits(z_pbuf, shift, BITS);
						if (b & MASK1)
							d[bFlip] -= (b & MASK2) << SHIFT;
						else
							d[bFlip] += (b & MASK2) << SHIFT;
					}
				}

				// 1-6 : 通常の差分
				else if (type != 0) {
					int BITS, SHIFT;
					if (nwaHed.CmpLevel >= 3) {
						BITS = 3 + nwaHed.CmpLevel;
						SHIFT = 1 + type;
					}
					else {
						BITS = 5 - nwaHed.CmpLevel;
						SHIFT = 2 + type + nwaHed.CmpLevel;
					}
					const int MASK1 = (1 << (BITS - 1));
					const int MASK2 = (1 << (BITS - 1)) - 1;
					int b = getbits(z_pbuf, shift, BITS);
					if (b & MASK1)
						d[bFlip] -= (b & MASK2) << SHIFT;
					else
						d[bFlip] += (b & MASK2) << SHIFT;
				}

				// type == 0
				else
				{
					if( bRLE )
					{
						// ランレングス圧縮

						int					nRunLength = getbits(z_pbuf, shift, 1);

						if( nRunLength == 1 )
						{
							nRunLength = getbits(z_pbuf, shift, 2);

							if( nRunLength == 3 )
							{
								nRunLength = getbits(z_pbuf, shift, 8);
							}
						}

						// 前のデータと同じデータを書いていく

						for( int k = 0 ; k <= nRunLength ; k++ )
						{
							if( nwaHed.bits == 8 )
							{
								*pbuf++ = (BYTE)d[bFlip];
							}
							else
							{
								*(LPWORD)pbuf = d[bFlip];
								pbuf += 2;
							}

							if (nwaHed.channels == 2)
							{
								bFlip ^= 1;	// channel切り替え
							}
						}

						j += nRunLength;
						continue;
					}
				}

				if( nwaHed.bits == 8 )
				{
					*pbuf++ = (BYTE)d[bFlip];
				}
				else
				{
					*(LPWORD)pbuf = d[bFlip];
					pbuf += 2;
				}

				if( nwaHed.channels == 2 )
				{
					bFlip ^= 1;	// channel切り替え
				}
			}

			wav.Write(&buf[0], curblocksize);
		}
	}

	return TRUE;
}