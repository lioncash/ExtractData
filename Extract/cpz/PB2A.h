#pragma once

#include "PB.h"
#include "../../ArcFile.h"

class CPB2A final : public CPB
{
public:
	BOOL Decode(CArcFile* pclArc, void* pbtSrc, DWORD dwSrcSize);

private:
	void Decrypt(BYTE* pbtTarget, DWORD dwSize);

	bool Decode1(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decode2(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decode4(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decode5(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decode6(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);

	bool Decomp1(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decomp2(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decomp4(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decomp5(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp, const BYTE* pbtBase, DWORD dwFrameNumber);
	bool Decomp6(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp, const BYTE* pbtBase, DWORD dwFrameNumber);
};
