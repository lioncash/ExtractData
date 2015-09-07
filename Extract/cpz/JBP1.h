#pragma once

class CJBP1 final
{
public:
	void Decomp(BYTE* pbtDst, const BYTE* pbtSrc, WORD wBpp = 0, const BYTE* pbtAlpha = nullptr, DWORD dwAlphaSize = 0);

private:
	void DCT(BYTE* arg1, BYTE* arg2);
	int GetNBit(const BYTE*& lpin, DWORD code, DWORD& bit_buffer, DWORD& bit_remain);
	int MakeTree(BYTE* lp1, int size, DWORD* lp2);
	void YCC2RGB(BYTE* dc, BYTE* ac, short* Y, short* CbCr, int line);
};
