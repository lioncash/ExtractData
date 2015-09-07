#pragma once

class CYuris final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	BOOL MountYPF(CArcFile* pclArc);
	BOOL MountYMV(CArcFile* pclArc);

	BOOL DecodeYMV(CArcFile* pclArc);
};
