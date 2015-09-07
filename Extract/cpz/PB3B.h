#pragma once

#include "PB.h"
#include "../../ArcFile.h"

class CPB3B final : public CPB
{
public:
	using PB3B_DECRYPT = void (*)(BYTE*, DWORD, CArcFile*, const SFileInfo&);

	BOOL Decode(CArcFile* pclArc, void* pbtSrc, DWORD dwSrcSize, PB3B_DECRYPT pfnDecrypt);

private:
	void Decrypt(BYTE* pbtTarget, DWORD dwSize);

	BOOL Decode1(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decode3(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decode4(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decode5(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decode6(CArcFile* pclArc, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp, PB3B_DECRYPT pfnDecrypt);

	BOOL Decomp1(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decomp3(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decomp4(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decomp5(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
	BOOL Decomp6(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth, long lHeight, WORD wBpp);
};
