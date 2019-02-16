#pragma once

#include "ExtractBase.h"

class CNavel final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountPac(CArcFile* archive);
	bool MountWpd(CArcFile* archive);
};
