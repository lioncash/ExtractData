#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Image.h"
#include "QLIE.h"

BOOL CQLIE::Mount(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".pack"))
		return FALSE;

	// シグネチャ取得
	BYTE signature[16];
	pclArc->Seek(-28, FILE_END);
	pclArc->Read(signature, sizeof(signature));

	// シグネチャチェック
	if (memcmp(signature, "FilePackVer", 11) != 0) {
		pclArc->SeekHed();
		return FALSE;
	}

	// ファイル数取得
	DWORD ctFile;
	pclArc->Read(&ctFile, 4);

	// インデックスの位置取得
	QWORD index_ofs;
	pclArc->Read(&index_ofs, 8);

	// インデックスに移動
	pclArc->Seek(index_ofs, FILE_BEGIN);

	// インデックスサイズを求める(Ver2.0以降はハッシュサイズも含まれる)
	DWORD index_size = pclArc->GetArcSize() - 28 - index_ofs;

	// インデックス読み込み(Ver2.0以降はハッシュも含まれる)
	YCMemory<BYTE> index(index_size);
	pclArc->Read(&index[0], index_size);

	DWORD   seed;
	LPBYTE  pIndex = &index[0];
	YCString version;
	void (*DecryptFunc)(LPBYTE, DWORD, DWORD);

	// packのバージョンを確認
	if (memcmp(signature, "FilePackVer1.0", 14) == 0 || memcmp(signature, "FilePackVer2.0", 14) == 0) {
		// バージョン
		version = _T("FilePackVer1.0");
		// 復号関数セット
		DecryptFunc = DecryptFileName;
	}
	else if (memcmp(signature, "FilePackVer3.0", 14) == 0) {
		LPBYTE pIndex2 = pIndex;
		// Figure out how much data is TOC entries..
		for (int i = 0; i < (int)ctFile; i++) {
			WORD filename_len = *(LPWORD)pIndex2;
			pIndex2 += 2 + filename_len + 28;
		}
		// Compute the obfuscation seed from the CRC(?) of some hash data.
		LPBYTE hash_bytes = pIndex2 + 32 + *(LPDWORD)&pIndex2[28] + 36;
		seed = crc_or_something(hash_bytes, 256) & 0x0FFFFFFF;
		// バージョン
		version = _T("FilePackVer3.0");
		// 復号関数セット
		DecryptFunc = DecryptFileNameV3;
	}
	else {
		pclArc->SeekHed();
		return FALSE;
	}

	for (int i = 0; i < (int)ctFile; i++) {
		// ファイル名取得
		WORD FileNameLen = *(LPWORD)&pIndex[0];
		//pclArc->Read(&FileNameLen, 2);
		BYTE szFileName[_MAX_FNAME];
		memcpy(szFileName, &pIndex[2], FileNameLen);
		//pclArc->Read(szFileName, FileNameLen);
		szFileName[FileNameLen] = '\0';
		// ファイル名復号
		DecryptFunc(szFileName, FileNameLen, seed);

		pIndex += 2 + FileNameLen;

		//BYTE index[28];
		//pclArc->Read(&index, 28);

		// リストビューに追加
		SFileInfo infFile;
		infFile.name = (LPTSTR)szFileName;
		infFile.start = *(LPQWORD)&pIndex[0];
		infFile.sizeCmp = *(LPDWORD)&pIndex[8];
		infFile.sizeOrg = *(LPDWORD)&pIndex[12];
		infFile.end = infFile.start + infFile.sizeCmp;
		infFile.key = (version == _T("FilePackVer1.0")) ? *(LPDWORD)&pIndex[20] : seed;
		infFile.title = version;
		pclArc->AddFileInfo(infFile);

		pIndex += 28;
	}

	return TRUE;
}

BOOL CQLIE::Decode(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".pack"))
		return FALSE;

	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	// バッファ確保
	YCMemory<BYTE> z_buf(pInfFile->sizeCmp);
	YCMemory<BYTE> buf; // 圧縮ファイルのときのみresize()で解凍後のバッファを確保する(省メモリ設計)
	LPBYTE pBuf = &z_buf[0];

	// ファイル読み込み
	pclArc->Read(pBuf, pInfFile->sizeCmp);

	// 暗号化ファイルを復号
	if (pInfFile->title == _T("FilePackVer1.0")) {
		if (pInfFile->key)
			Decrypt(pBuf, pInfFile->sizeCmp, 0);
	}
	else if (pInfFile->title == _T("FilePackVer3.0"))
		DecryptV3(pBuf, pInfFile->sizeCmp, pInfFile->key);

	// 圧縮ファイルを解凍
	if (pInfFile->sizeCmp != pInfFile->sizeOrg) {
		buf.resize(pInfFile->sizeOrg);
		Decomp(&buf[0], pInfFile->sizeOrg, pBuf, pInfFile->sizeCmp);
		pBuf = &buf[0];
	}

	// 出力
	if (pInfFile->format == _T("BMP")) {
		LPBITMAPFILEHEADER fHed = (LPBITMAPFILEHEADER)&pBuf[0];
		LPBITMAPINFOHEADER iHed = (LPBITMAPINFOHEADER)&pBuf[14];
		// サイズ調整
		DWORD dstSize = pInfFile->sizeOrg - 54;
		if (((iHed->biWidth * (iHed->biBitCount >> 3) + 3) & 0xFFFFFFFC) * iHed->biHeight != dstSize)
			dstSize -= 2;
		// 出力
		CImage image;
		image.Init(pclArc, iHed->biWidth, iHed->biHeight, iHed->biBitCount, &pBuf[54], fHed->bfOffBits - 54);
		image.Write(&pBuf[fHed->bfOffBits], dstSize);
	}
	else if( pInfFile->format == _T("B") )
	{
		// *.bファイル

		if( !DecodeB( pclArc, pBuf, pInfFile->sizeOrg ) )
		{
			// 未対応

			pclArc->OpenFile();
			pclArc->WriteFile( pBuf, pInfFile->sizeOrg );
		}
	}
	else {
		pclArc->OpenFile();
		pclArc->WriteFile(pBuf, pInfFile->sizeOrg);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	*.bファイルの展開

BOOL	CQLIE::DecodeB(
	CArcFile*			pclArc,							// アーカイブ
	BYTE*				pbtSrc,							// bファイル
	DWORD				dwSrcSize						// bファイルサイズ
	)
{
	if( memcmp( &pbtSrc[0], "ABMP7", 5 ) == 0 )
	{
		// ABMP7

		return	DecodeABMP7( pclArc, pbtSrc, dwSrcSize );
	}

	if( memcmp( &pbtSrc[0], "abmp", 4 ) == 0 )
	{
		// abmp10

		return	DecodeABMP10( pclArc, pbtSrc, dwSrcSize );
	}

	// 未対応

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	ABMP7の展開

BOOL	CQLIE::DecodeABMP7(
	CArcFile*			pclArc,							// アーカイブ
	BYTE*				pbtSrc,							// bファイル
	DWORD				dwSrcSize,						// bファイルサイズ
	DWORD*				pdwSrcIndex,					// 全体のbファイルに対するインデックス(再帰呼び出し後に進んだ分を加算する必要がある)
	const YCString&		clsBFileName					// 今までのbファイル名(再起呼び出しを行うたびにbファイル名を繋げていく)
	)
{
	DWORD				dwSrcIndex = 0;

	if( (dwSrcIndex + 16) > dwSrcSize )
	{
		// 終了

		return	FALSE;
	}

	if( memcmp( &pbtSrc[dwSrcIndex], "ABMP7", 5 ) != 0 )
	{
		// 未対応

		return	FALSE;
	}

	dwSrcIndex += 12;

	// 不要なデータサイズの取得

	DWORD				dwUnKnownDataSize = *(DWORD*) &pbtSrc[dwSrcIndex];

	dwSrcIndex += 4;

	// 不要なデータをスキップ

	dwSrcIndex += dwUnKnownDataSize;

//-- 初回読み込み ------------------------------------------------------------------------

	// ファイルサイズの取得

	DWORD				dwFileSize = *(DWORD*) &pbtSrc[dwSrcIndex];

	dwSrcIndex += 4;

	// 拡張子の取得

	YCString			clsFileExt = GetExtension( &pbtSrc[dwSrcIndex] );

	// 出力

	if( clsFileExt == _T(".bmp") )
	{
		// BITMAP

		CImage				clImage;

		clImage.Init( pclArc, &pbtSrc[dwSrcIndex] );
		clImage.Write( dwFileSize );
		clImage.Close();
	}
	else
	{
		// その他

		pclArc->OpenFile( clsFileExt );
		pclArc->WriteFile( &pbtSrc[dwSrcIndex], dwFileSize );
		pclArc->CloseFile();
	}

	dwSrcIndex += dwFileSize;

//-- 2回目以降の読み込み -----------------------------------------------------------------

	while( dwSrcIndex < dwSrcSize )
	{
		// ファイル名の長さの取得

		DWORD				dwFileNameLength = pbtSrc[dwSrcIndex];

		dwSrcIndex += 1;

		// ファイル名の取得

		char				szFileName[_MAX_FNAME];

		memcpy( szFileName, &pbtSrc[dwSrcIndex], dwFileNameLength );

		szFileName[dwFileNameLength] = '\0';

		dwSrcIndex += 31;

		// ファイルサイズの取得

		dwFileSize = *(DWORD*) &pbtSrc[dwSrcIndex];

		dwSrcIndex += 4;

		if( dwFileSize == 0 )
		{
			// ファイルサイズが0

			continue;
		}

		// 拡張子の取得

		clsFileExt = GetExtension( &pbtSrc[dwSrcIndex] );

		// 「bファイル名_ファイル名.拡張子」という名前で書き込む

		TCHAR				szWork[_MAX_FNAME];

		_stprintf( szWork, _T("_%s%s"), szFileName, clsFileExt );

		// 出力

		if( clsFileExt == _T(".bmp") )
		{
			// BITMAP

			CImage				clImage;

			clImage.Init( pclArc, &pbtSrc[dwSrcIndex], szWork );
			clImage.Write( dwFileSize );
			clImage.Close();
		}
		else
		{
			// その他

			pclArc->OpenFile( szWork );
			pclArc->WriteFile( &pbtSrc[dwSrcIndex], dwFileSize );
			pclArc->CloseFile();
		}

		dwSrcIndex += dwFileSize;
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	abmp10～12の展開

BOOL	CQLIE::DecodeABMP10(
	CArcFile*			pclArc,							// アーカイブ
	BYTE*				pbtSrc,							// bファイル
	DWORD				dwSrcSize,						// bファイルサイズ
	DWORD*				pdwSrcIndex,					// 全体のbファイルに対するインデックス(再帰呼び出し後に進んだ分を加算する必要がある)
	const YCString&		clsBFileName					// 今までのbファイル名(再起呼び出しを行うたびにbファイル名を繋げていく)
	)
{
	static DWORD						dwDstFiles;
	static std::vector<SFileNameInfo>	vtFileNameList;

	if( pdwSrcIndex == NULL )
	{
		// 初回の呼び出し

		dwDstFiles = 0;
		vtFileNameList.clear();
	}

	DWORD				dwSrcIndex = 0;

	if( (dwSrcIndex + 16) > dwSrcSize )
	{
		// 終了

		return	FALSE;
	}

	if( memcmp( &pbtSrc[dwSrcIndex], "abmp", 4 ) != 0 )
	{
		// 未対応

		return	FALSE;
	}

	dwSrcIndex += 16;

	if( memcmp( &pbtSrc[dwSrcIndex], "abdata", 6 ) != 0 )
	{
		// 未対応

		return	FALSE;
	}

	dwSrcIndex += 16;

	// 不要なデータサイズの取得

	DWORD				dwUnKnownDataSize = *(DWORD*) &pbtSrc[dwSrcIndex];

	dwSrcIndex += 4;

	// 不要なデータをスキップ

	dwSrcIndex += dwUnKnownDataSize;

	while( dwSrcIndex < dwSrcSize )
	{
		if( (dwSrcIndex + 16) > dwSrcSize )
		{
			// 終了

			break;
		}

		if( memcmp( &pbtSrc[dwSrcIndex], "abimage", 7 ) == 0 )
		{
			// abimage10
		}
		else if( memcmp( &pbtSrc[dwSrcIndex], "absound", 7 ) == 0 )
		{
			// absound10
		}
		else
		{
			// 未対応

			return	FALSE;
		}

		dwSrcIndex += 16;

		// ファイル数の取得

		DWORD				dwFiles = pbtSrc[dwSrcIndex];

		dwSrcIndex += 1;

		for( DWORD i = 0 ; i < dwFiles ; i++ )
		{
			if( (dwSrcIndex + 16) > dwSrcSize )
			{
				// 終了

				break;
			}

			DWORD				dwDatVersion;

			if( memcmp( &pbtSrc[dwSrcIndex], "abimgdat10", 10 ) == 0 )
			{
				// abimgdat10

				dwDatVersion = ABIMGDAT10;
			}
			else if( memcmp( &pbtSrc[dwSrcIndex], "abimgdat11", 10 ) == 0 )
			{
				// abimgdat11

				dwDatVersion = ABIMGDAT11;
			}
			else if( memcmp( &pbtSrc[dwSrcIndex], "absnddat10", 10 ) == 0 )
			{
				// absnddat10

				dwDatVersion = ABSNDDAT10;
			}
			else if( memcmp( &pbtSrc[dwSrcIndex], "absnddat11", 10 ) == 0 )
			{
				// absnddat11

				dwDatVersion = ABSNDDAT11;
			}
			else
			{
				// 未対応

				return	FALSE;
			}

			dwSrcIndex += 16;

			// ファイル名の長さの取得

			DWORD				dwFileNameLength = *(WORD*) &pbtSrc[dwSrcIndex];

			dwSrcIndex += 2;

			// ファイル名の取得

			YCString			clsFileName( (char*) &pbtSrc[dwSrcIndex], dwFileNameLength );

			dwSrcIndex += dwFileNameLength;

			// 不明なデータをスキップ

			if( (dwDatVersion == ABIMGDAT11) || (dwDatVersion == ABSNDDAT11) )
			{
				WORD				wLength = *(WORD*) &pbtSrc[dwSrcIndex];

				dwSrcIndex += 2 + wLength;
			}

			dwSrcIndex += 1;

			// ファイルサイズの取得

			DWORD				dwFileSize = *(DWORD*) &pbtSrc[dwSrcIndex];

			dwSrcIndex += 4;

			if( dwFileSize == 0 )
			{
				// ファイルサイズが0

				continue;
			}

			// 拡張子の取得

			YCString			clsFileExt = GetExtension( &pbtSrc[dwSrcIndex] );

			if( clsFileExt == _T(".b") )
			{
				// abmp10

				YCString		clsWork;

				if( clsFileName == _T("") )
				{
					clsWork = clsBFileName;
				}
				else
				{
					clsWork = clsBFileName + _T("_") + clsFileName;
				}

				DecodeABMP10( pclArc, &pbtSrc[dwSrcIndex], dwFileSize, &dwSrcIndex, clsWork );

				continue;
			}

			// '/'を'_'に置換(全角文字を考慮)

			clsFileName.Replace( _T('/'), _T('_') );

			// パスに使えない文字を消去

			EraseNotUsePathWord( clsFileName );

			// 拡張子を付加

			clsFileName.RenameExtension( clsFileExt );	// ファイル名にpsd等の拡張子が付いている場合があるのでリネームで対処

			// 「bファイル名_ファイル名.拡張子」という名前で書き込む

			TCHAR				szWork[_MAX_FNAME];

			if( dwFileNameLength == 0 )
			{
				// ファイル名がない(拡張子だけ付ける)

				_stprintf( szWork, _T("%s%s"), clsBFileName, clsFileName );
			}
			else
			{
				_stprintf( szWork, _T("%s_%s"), clsBFileName, clsFileName );
			}

			// 同じファイル名が存在していないか確認

			for( size_t uIndex = 0 ; uIndex < vtFileNameList.size() ; uIndex++ )
			{
				if( vtFileNameList[uIndex].clsFileName == szWork )
				{
					// 同じファイル名

					vtFileNameList[uIndex].dwCount++;

					// 上書きが発生しないようにする

					TCHAR				szWork2[256];

					_stprintf( szWork2, _T("_%d%s"), vtFileNameList[uIndex].dwCount, clsFileName.GetFileExt() );

					PathRenameExtension( szWork, szWork2 );

					// 同じファイル名が存在していないか確認するためにループカウンタをリセット

					uIndex = (size_t) -1;
				}
			}

			// ファイル名を登録

			SFileNameInfo		stFileNameInfo;

			stFileNameInfo.clsFileName = szWork;
			stFileNameInfo.dwCount = 1;

			vtFileNameList.push_back( stFileNameInfo );

			// 出力

			if( clsFileExt == _T(".bmp") )
			{
				// BITMAP

				CImage				clImage;

				clImage.Init( pclArc, &pbtSrc[dwSrcIndex], szWork );
				clImage.Write( dwFileSize );
				clImage.Close();
			}
			else
			{
				// その他

				pclArc->OpenFile( szWork );
				pclArc->WriteFile( &pbtSrc[dwSrcIndex], dwFileSize );
				pclArc->CloseFile();
			}

			dwDstFiles++;

			dwSrcIndex += dwFileSize;
		}
	}

	if( pdwSrcIndex != NULL )
	{
		// 再帰呼び出しが行われている

		*pdwSrcIndex += dwSrcIndex;
	}
	else
	{
		if( dwDstFiles == 0 )
		{
			// ファイルが1つも出力されていない

			return	FALSE;
		}
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	拡張子の取得

YCString	CQLIE::GetExtension(
	BYTE*				pbtSrc							// 入力バッファ
	)
{
	YCString			clsFileExt;

	if( memcmp( &pbtSrc[0], "\x89PNG", 4 ) == 0 )
	{
		// PNG

		clsFileExt = _T(".png");
	}
	else if( memcmp( &pbtSrc[0], "\xFF\xD8\xFF\xE0", 4 ) == 0 )
	{
		// JPEG

		clsFileExt = _T(".jpg");
	}
	else if( memcmp( &pbtSrc[0], "BM", 2 ) == 0 )
	{
		// BITMAP

		clsFileExt = _T(".bmp");
	}
	else if( memcmp( &pbtSrc[0], "OggS", 4 ) == 0 )
	{
		// Ogg Vorbis

		clsFileExt = _T(".ogg");
	}
	else if( memcmp( &pbtSrc[0], "RIFF", 4 ) == 0 )
	{
		// WAVE

		clsFileExt = _T(".wav");
	}
	else if( memcmp( &pbtSrc[0], "abmp", 4 ) == 0 )
	{
		// abmp10

		clsFileExt = _T(".b");
	}

	return	clsFileExt;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	パスに使えない文字を消去

void	CQLIE::EraseNotUsePathWord(
	YCString&				clsPath							// パス
	)
{
	static const TCHAR	aszNotUsePathWord[] =
	{
		_T(':'), _T(','), _T(';'), _T('*'), _T('?'), _T('\"'), _T('<'), _T('>'), _T('|')
	};

	for( DWORD dwIndex = 0 ; dwIndex < _countof( aszNotUsePathWord ) ; dwIndex++ )
	{
		clsPath.Remove( aszNotUsePathWord[dwIndex] );
	}
}


// DecryptFileName, Decrypt, Decompは、
// 十一寒月氏が作成・公開しているgageのソースコードを参考にして作成しました。

void CQLIE::DecryptFileName(LPBYTE in, DWORD size, DWORD seed)
{
	for (int i = 1; i <= (int)size; i++)
		*in++ ^= (i + (i ^ ((0xFA + size) & 0xFF))) & 0xFF;
}

void CQLIE::Decrypt(LPBYTE buf, DWORD buf_len, DWORD seed)
{
	DWORD key1 = buf_len ^ 0xFEC9753E;
	DWORD key2 = key1;
	DWORD x1 = 0xA73C5F9D;
	DWORD x2 = x1;
	DWORD size = buf_len >> 3;

	for (int i = 0; i < (int)size; i++) {
		x1 = (x1 + 0xCE24F523) ^ key1;
		x2 = (x2 + 0xCE24F523) ^ key2;
		key1 = ((LPDWORD)buf)[i*2+0] ^= x1;
		key2 = ((LPDWORD)buf)[i*2+1] ^= x2;
	}
}

void CQLIE::Decomp(LPBYTE dst, DWORD dstSize, LPBYTE src, DWORD srcSize)
{
	BYTE buf[1024];
	LPBYTE pdst = dst;
	LPBYTE psrc = src + 12;

	if (*(LPDWORD)src != 0xFF435031)
		return;

	for (int i = 0; i < 256; i++)
		buf[i] = i;

	BYTE x = src[4];

	while ((psrc - src) < ((int)srcSize - 12)) {
		memcpy(&buf[256], buf, 256);

		for (int d = 0; d <= 0xFF; ) {
			int c = *psrc++;
			if (c > 0x7F) {
				d += c - 0x7F;
				c = 0;
			}
			if (d > 0xff)
				break;

			for (int i = c+1; i > 0; i--) {
				buf[256 + d] = *psrc++;
				if (buf[256 + d] != d)
					buf[512 + d] = *psrc++;
				d++;
			}
		}

		int c;
		if (x & 1) {
			c = *(LPWORD)psrc;
			psrc += 2;
		}
		else {
			c = *(LPDWORD)psrc;
			psrc += 4;
		}

		for (int n = 0; ; ) {
			int d;
			if (n)
				d = buf[--n + 768];
			else {
				if (c == 0) break;
				c--;
				d = *psrc++;
			}
			if (buf[256 + d] == d) {
				if (pdst - dst > (int)dstSize)
					return;
				*pdst++ = d;
			}
			else {
				buf[768 + n++] = buf[512 + d];
				buf[768 + n++] = buf[256 + d];
			}
		}
	}
}

// padw, crc_or_something, DecryptFileNameV3, DecryptV3は、
// asmodean氏が作成・公開しているexfp3のソースコードを参考にして作成しました。

// Emulate mmx padw instruction
QWORD CQLIE::padw(QWORD a, QWORD b)
{
	LPWORD a_words = (LPWORD)&a;
	LPWORD b_words = (LPWORD)&b;

	QWORD ret = 0;
	LPWORD r_words = (LPWORD)&ret;

	r_words[0] = a_words[0] + b_words[0];
	r_words[1] = a_words[1] + b_words[1];
	r_words[2] = a_words[2] + b_words[2];
	r_words[3] = a_words[3] + b_words[3];

	return ret;
}

DWORD CQLIE::crc_or_something(LPBYTE buff, DWORD len)
{
	QWORD   key    = 0;
	QWORD   result = 0;
	LPQWORD p      = (LPQWORD)buff;
	LPQWORD end    = p + (len / 8);

	while (p < end) {
		key = padw(key, 0x0307030703070307);

		QWORD   temp       = *p++ ^ key;
		LPDWORD temp_sects = (LPDWORD)&temp;

		result = padw(result, temp);
	}

	result ^= (result >> 32);

	return (DWORD)(result & 0xFFFFFFFF);
}

void CQLIE::DecryptFileNameV3(LPBYTE buff, DWORD len, DWORD seed)
{
	DWORD mutator = seed ^ 0x3E;
	DWORD key     = (mutator + len) & 0xFF;

	for (int i = 1; i <= (int)len; i++)
		buff[i - 1] ^= ((i ^ key) & 0xFF) + i;
}

void CQLIE::DecryptV3(LPBYTE buff, DWORD len, DWORD seed)
{
	QWORD   key       = 0xA73C5F9DA73C5F9D;
	LPDWORD key_words = (LPDWORD)&key;
	QWORD   mutator   = (seed + len) ^ 0xFEC9753E;

	mutator = (mutator << 32) | mutator;

	LPQWORD p   = (LPQWORD)buff;
	LPQWORD end = p + (len / 8);

	while (p < end) {
		// Emulate mmx padd instruction
		key_words[0] += 0xCE24F523;
		key_words[1] += 0xCE24F523;

		key     ^= mutator;
		mutator  = *p++ ^= key;
	}
}