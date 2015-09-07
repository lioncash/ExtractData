#pragma once

class CYkc final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	BOOL DecodeYKS(CArcFile* pclArc);
	BOOL DecodeYKG(CArcFile* pclArc);
};
