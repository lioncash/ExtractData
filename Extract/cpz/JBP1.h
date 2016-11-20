#pragma once

class CJBP1 final
{
public:
	void Decomp(u8* pbtDst, const u8* pbtSrc, u16 wBpp = 0, const u8* pbtAlpha = nullptr, u32 dwAlphaSize = 0);

private:
	void DCT(u8* arg1, u8* arg2);
	s32 GetNBit(const u8*& lpin, u32 code, u32& bit_buffer, u32& bit_remain);
	s32 MakeTree(u8* lp1, s32 size, u32* lp2);
	void YCC2RGB(u8* dc, u8* ac, s16* Y, s16* CbCr, s32 line);
};
