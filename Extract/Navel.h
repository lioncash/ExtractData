#pragma once

class CNavel : public CExtractBase {
protected:
	// WPDフォーマット
	struct FormatWPD {
		BYTE	ID[4];		// " DPW"
		DWORD	type;		// 不明
		DWORD	dummy2;		// 不明
		DWORD	ChunkByte;	// チャンクのバイト数
		DWORD	freq;		// サンプリングレート
		DWORD	DataSize;	// データサイズ
		WORD	FormatID;	// フォーマットID
		WORD	channels;	// チャンネル数
		DWORD	freq2;		// サンプリングレート(何故か2つある)
		DWORD	bps;		// データ速度(Byte/sec)
		WORD	BlockSize;	// ブロックサイズ(Byte/sample * チャンネル数)
		WORD	bits;		// サンプルあたりのビット数(bit/sample)
		DWORD	dummy3;		// 不明
	};

public:
	BOOL Mount(CArcFile* pclArc);
	BOOL MountPac(CArcFile* pclArc);
	BOOL MountWpd(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);
};