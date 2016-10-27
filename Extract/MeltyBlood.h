#pragma once

class CMeltyBlood final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	void Decrypt(CArcFile* pclArc);
};
