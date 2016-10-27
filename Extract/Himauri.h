#pragma once

class CHimauri final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	bool MountHim4(CArcFile* pclArc);
	bool MountHim5(CArcFile* pclArc);

	void Decomp(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize);
	bool Compose(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtBase, DWORD dwBaseSize, const BYTE* pbtDiff, DWORD dwDiffSize);
};
