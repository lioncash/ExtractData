#pragma once

class CVA : public CExtractBase {
private:
	BOOL MountNwa(CArcFile* pclArc);
	BOOL MountNwk(CArcFile* pclArc);
	BOOL MountOvk(CArcFile* pclArc);
	BOOL DecodeNwa(CArcFile* pclArc);

	inline int getbits(LPBYTE& data, int& shift, int bits);

protected:
	// NWAフォーマット
	struct NWAHed {
		WORD	channels;		// チャンネル数
		WORD	bits;			// サンプルあたりのビット数(bit/sample)
		DWORD	freq;			// サンプリングレート
		DWORD	CmpLevel;		// 圧縮レベル
		DWORD	dummy1;			// 不明
		DWORD	blocks;			// ブロック数
		DWORD	DataSize;		// 展開後の波形データのバイト数
		DWORD	CompFileSize;	// 圧縮時のファイルサイズ
		DWORD	samples;		// 展開後のデータ数
		DWORD	BlockSize;		// データ1ブロックを展開した時のサンプル単位のデータ数
		DWORD	RestSize;		// 最終ブロックを展開した時のサンプル単位のデータ数
		DWORD	dummy2;			// 不明
	};

	// NWKインデックスヘッダ
	struct IndexNwk {
		DWORD	size;
		DWORD	start;
		DWORD	dummy;
	};

public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);
};