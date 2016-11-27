#pragma once

#include "ExtractBase.h"

class CTH2 final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountKCAP(CArcFile* archive);
	bool MountLAC(CArcFile* archive);
	bool MountDpl(CArcFile* archive);
	bool MountWMV(CArcFile* archive);

	bool DecodeWMV(CArcFile* archive);
	bool DecodeEtc(CArcFile* archive);
};
