#pragma once

#include "ExtractBase.h"

class CYuris final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool MountYPF(CArcFile* archive);
	bool MountYMV(CArcFile* archive);

	bool DecodeYMV(CArcFile* archive);
};
