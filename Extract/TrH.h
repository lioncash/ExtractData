#pragma once

class CTrH : public CExtractBase {
protected:
	// PXフォーマット
	struct PXHed {
		BYTE	pxID[4];		// "fPX "
		DWORD	FileSize;		// ファイルサイズ
		BYTE	CtrkID[4];		// "cTRK"
		DWORD	DataSize;		// 波形データのバイト数
		DWORD	dummy1;			// 不明
		DWORD	dummy2;			// 不明
		DWORD	dummy3;			// 不明
		DWORD	freq;			// サンプリングレート
		WORD	dummy4;			// 不明
		WORD	channels;		// チャンネル数
		DWORD	ChunkByte;		// チャンクのバイト数
		WORD	bits;			// サンプルあたりのビット数(bit/sample)
		WORD	FormatID;		// フォーマットID
	};

public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);
};