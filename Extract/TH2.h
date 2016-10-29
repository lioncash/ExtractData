#pragma once

#include "../ExtractBase.h"

class CTH2 final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	bool MountKCAP(CArcFile* pclArc);
	bool MountLAC(CArcFile* pclArc);
	bool MountDpl(CArcFile* pclArc);
	bool MountWMV(CArcFile* pclArc);

	bool DecodeWMV(CArcFile* pclArc);
	bool DecodeEtc(CArcFile* pclArc);
};
