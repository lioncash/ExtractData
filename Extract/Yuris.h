#pragma once

class CYuris final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	bool MountYPF(CArcFile* pclArc);
	bool MountYMV(CArcFile* pclArc);

	bool DecodeYMV(CArcFile* pclArc);
};
