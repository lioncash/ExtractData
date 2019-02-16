#pragma once

#include "ExtractBase.h"

class CTaskForce final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountDat(CArcFile* archive);
	bool MountTlz(CArcFile* archive);
	bool MountBma(CArcFile* archive);

	bool DecodeTlz(CArcFile* archive);
	bool DecodeBma(CArcFile* archive);
	bool DecodeTGA(CArcFile* archive);
};
