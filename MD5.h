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

	SMD5    Calculate(LPCTSTR file_path);
	SMD5    Calculate(const void* src, size_t src_size, const u32* initialize = nullptr, bool alignment = false);

	size_t  CalculatePadding(size_t size);
	void    AppendPadding(void* src, size_t src_size, u32 padding);

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
	void    ValueToStr(char* md5_dst, u32 md5);
	void    MD5ToStrings(char* md5_dst, const u32* md5);

private:
	SMD5 m_md5_value;
	std::array<u32, 16> m_x;
};
