#pragma once

class CTH2 final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	bool MountKCAP(CArcFile* pclArc);
	bool MountLAC(CArcFile* pclArc);
	bool MountDpl(CArcFile* pclArc);
	bool MountWMV(CArcFile* pclArc);

	bool DecodeWMV(CArcFile* pclArc);
	bool DecodeEtc(CArcFile* pclArc);
};
