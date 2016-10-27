#pragma once

class CYkc final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	bool DecodeYKS(CArcFile* pclArc);
	bool DecodeYKG(CArcFile* pclArc);
};
