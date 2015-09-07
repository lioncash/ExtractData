#pragma once

class CHimauri final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL MountHim4(CArcFile* pclArc);
	BOOL MountHim5(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc) override;

private:
	void Decomp(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize);
	BOOL Compose(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtBase, DWORD dwBaseSize, const BYTE* pbtDiff, DWORD dwDiffSize);
};
