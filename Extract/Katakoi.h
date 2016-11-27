#pragma once

#include "ExtractBase.h"

class CKatakoi final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	bool MountIar(CArcFile* pclArc);
	bool MountWar(CArcFile* pclArc);

	bool DecodeIar(CArcFile* pclArc);
	bool DecodeWar(CArcFile* pclArc);

	bool GetNameIndex(CArcFile* pclArc, YCMemory<BYTE>& clmbtSec, DWORD& dwNameIndex);
	bool GetPathToSec(LPTSTR pszPathToSec, const YCString& strPathToArc);

	void GetBit(LPBYTE& pbySrc, DWORD& dwFlags);
	bool DecompImage(LPBYTE pbyDst, DWORD dwDstSize, LPBYTE pbySrc, DWORD dwSrcSize);

	bool Compose(LPBYTE pbyDst, DWORD dwDstSize, LPBYTE pbySrc, DWORD dwSrcSize, long lWidthForDst, long lWidthForSrc, WORD wBpp);
	bool DecodeCompose(CArcFile* pclArc, LPBYTE pbyDiff, DWORD dwDiffSize, long lWidthForDiff, long lHeightForDiff, WORD wBppForDiff);
};
