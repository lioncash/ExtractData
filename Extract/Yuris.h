#pragma once

#include "../ExtractBase.h"

class CYuris final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	bool MountYPF(CArcFile* pclArc);
	bool MountYMV(CArcFile* pclArc);

	bool DecodeYMV(CArcFile* pclArc);
};
