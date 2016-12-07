#pragma once

#include "ExtractBase.h"

class CMeltyBlood final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	void Decrypt(CArcFile* archive);
};
