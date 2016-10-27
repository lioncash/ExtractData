#pragma once

#include "PB.h"
#include "../../ArcFile.h"

class CPB3B final : public CPB
{
public:
	using PB3B_DECRYPT = void (*)(BYTE*, DWORD, CArcFile*, const SFileInfo&);

	bool Decode(CArcFile* pclArc, void* pbtSrc, DWORD dwSrcSize, PB3B_DECRYPT pfnDecrypt);

private:
	void Decrypt(BYTE* pbtTarget, DWORD dwSize);

	bool Decode1(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decode3(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decode4(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decode5(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decode6(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp, PB3B_DECRYPT pfnDecrypt);

	bool Decomp1(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decomp3(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decomp4(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decomp5(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	bool Decomp6(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
};
