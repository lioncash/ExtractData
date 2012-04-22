#pragma once

class CFateFD : public CExtractBase {
protected:
	// デコード情報
	struct DecodeInfo {
		BYTE key;
		DWORD size;
	};

public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);
};