#pragma once

class CMeltyBlood : public CExtractBase {
public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);

	void Decrypt(CArcFile* pclArc);
};