#pragma once

class CCyc : public CExtractBase {
public:
	// マウント
	BOOL Mount(CArcFile* pclArc);
	// gtbを参照し、gpkをマウント(dwqがアーカイブされている)
	BOOL MountGpk(CArcFile* pclArc);
	// vtbを参照し、vpkをマウント(wgqがアーカイブされている)
	BOOL MountVpk(CArcFile* pclArc);
	// dwqをマウント(BMP: PACKTYPE=0, PACKBMP+MASK: PACKTYPE=3A, JPEG: PACKTYPE=5, JPEG+MASK: PACKTYPE=7A)
	BOOL MountDwq(CArcFile* pclArc);
	// wgqをマウント(OGG: PACKTYPE=6)
	BOOL MountWgq(CArcFile* pclArc);
	// vawをマウント(WAV: PACKTYPE=0, RIFF OGG: PACKTYPE=2)
	BOOL MountVaw(CArcFile* pclArc);
	// xtxをマウント(テキストデータ)
	BOOL MountXtx(CArcFile* pclArc);
	// fxfをマウント(xtxをビット反転したデータ)
	BOOL MountFxf(CArcFile* pclArc);

	// デコード
	BOOL Decode(CArcFile* pclArc);
	// dwqをデコード
	BOOL DecodeDwq(CArcFile* pclArc);
	// wgqをデコード
	BOOL DecodeWgq(CArcFile* pclArc);
	// vawをデコード
	BOOL DecodeVaw(CArcFile* pclArc);
	// xtxをデコード
	BOOL DecodeXtx(CArcFile* pclArc);
	// fxfをデコード
	BOOL DecodeFxf(CArcFile* pclArc);

	// RLEをdstに解凍
	void DecompRLE(LPBYTE dst, LPBYTE src, LONG width, LONG height);
	// RGBをBGRに並べ替える
	void RGBtoBGR(LPBYTE buf, LONG widht, LONG height);
};