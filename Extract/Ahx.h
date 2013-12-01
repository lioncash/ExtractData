#pragma once

class CAhx : public CExtractBase
{
private:
	int getbits(LPBYTE& src, int& bit_data, int& bit_rest, int bits);
	void dct(double* src, double* dst0, double* dst1);

public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);

	void Decode(CArcFile* pclArc, LPBYTE ahx_buf, DWORD ahx_buf_len);
	int Decompress(LPBYTE dst, LPBYTE src, int srclen);
};
