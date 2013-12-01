#pragma once

class CYkc : public CExtractBase
{
private:
	BOOL DecodeYKS(CArcFile* pclArc);
	BOOL DecodeYKG(CArcFile* pclArc);

public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);
};
