#pragma once

class CTH2 final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	BOOL MountKCAP(CArcFile* pclArc);
	BOOL MountLAC(CArcFile* pclArc);
	BOOL MountDpl(CArcFile* pclArc);
	BOOL MountWMV(CArcFile* pclArc);

	BOOL DecodeWMV(CArcFile* pclArc);
	BOOL DecodeEtc(CArcFile* pclArc);
};
