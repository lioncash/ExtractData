#pragma once

struct SMD5
{
	char szABCD[32];
	u32  adwABCD[4];
};

class CMD5
{
public:
	CMD5();

	SMD5    Calculate(LPCTSTR pszPathToFile);
	SMD5    Calculate(const void* src, u32 src_size, const u32* initialize = nullptr, bool alignment = false);

	u32     CalculatePadding(u32 size);
	void    AppendPadding(void* src, u32 src_size, u32 padding);

protected:
	u32     CalculateSub1(u32 x, u32 y, u32 z);
	u32     CalculateSub2(u32 x, u32 y, u32 z);
	u32     CalculateSub3(u32 x, u32 y, u32 z);
	u32     CalculateSub4(u32 x, u32 y, u32 z);
	void    CalculateSub5(u32& a, u32 b, u32 c, u32 d, u32 k, u32 s, u32 i);
	void    CalculateSub6(u32& a, u32 b, u32 c, u32 d, u32 k, u32 s, u32 i);
	void    CalculateSub7(u32& a, u32 b, u32 c, u32 d, u32 k, u32 s, u32 i);
	void    CalculateSub8(u32& a, u32 b, u32 c, u32 d, u32 k, u32 s, u32 i);

	u32     RotateLeft(u32 value, u32 shift);
	void    ValueToStr(LPSTR md5_dst, u32 md5);
	void    MD5ToStrings(LPSTR md5_dst, const u32* md5);

private:
	int   m_anTable[65];
	SMD5  m_stmd5Value;
	u32   m_adwX[16];
};
