#pragma once

#include "../ExtractBase.h"

class CAhx final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

	void Decode(CArcFile* archive, LPBYTE ahx_buf, DWORD ahx_buf_len);
	int Decompress(LPBYTE dst, LPBYTE src, int srclen);

private:
	int getbits(LPBYTE& src, int& bit_data, int& bit_rest, int bits);
	void dct(const double* src, double* dst0, double* dst1);
};
