#pragma once

#include "../ExtractBase.h"

class CMeltyBlood final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	void Decrypt(CArcFile* pclArc);
};
