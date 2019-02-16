#pragma once

#include "ExtractBase.h"

class CSpitan final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;

private:
	bool MountSound(CArcFile* archive);
	bool MountGraphic1(CArcFile* archive);
	bool MountGraphic2(CArcFile* archive);
	bool MountGraphic3(CArcFile* archive);
};
