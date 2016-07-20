#pragma once

class CAhx final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

	void Decode(CArcFile* pclArc, LPBYTE ahx_buf, DWORD ahx_buf_len);
	int Decompress(LPBYTE dst, LPBYTE src, int srclen);

private:
	int getbits(LPBYTE& src, int& bit_data, int& bit_rest, int bits);
	void dct(const double* src, double* dst0, double* dst1);
};
