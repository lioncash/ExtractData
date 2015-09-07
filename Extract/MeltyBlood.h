#pragma once

class CMeltyBlood final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

	void Decrypt(CArcFile* pclArc);
};
