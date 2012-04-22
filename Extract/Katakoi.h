#pragma once

class CKatakoi : public CExtractBase {
private:
	BOOL MountIar(CArcFile* pclArc);
	BOOL MountWar(CArcFile* pclArc);

	BOOL DecodeIar(CArcFile* pclArc);
	BOOL DecodeWar(CArcFile* pclArc);

	BOOL GetNameIndex(CArcFile* pclArc, YCMemory<BYTE>& clmbtSec, DWORD& dwNameIndex);
	BOOL GetPathToSec(LPTSTR pszPathToSec, const YCString& strPathToArc);

	void GetBit(LPBYTE& pbySrc, DWORD& dwFlags);
	BOOL DecompImage(LPBYTE pbyDst, DWORD dwDstSize, LPBYTE pbySrc, DWORD dwSrcSize);

	BOOL Compose(LPBYTE pbyDst, DWORD dwDstSize, LPBYTE pbySrc, DWORD dwSrcSize, long lWidthForDst, long lWidthForSrc, WORD wBpp);
	BOOL DecodeCompose(CArcFile* pclArc, LPBYTE pbyDiff, DWORD dwDiffSize, long lWidthForDiff, long lHeightForDiff, WORD wBppForDiff);

public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);
};
