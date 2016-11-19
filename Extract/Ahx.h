#pragma once

#include "../ExtractBase.h"

class CAhx final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

	void Decode(CArcFile* archive, const u8* ahx_buf, size_t ahx_buf_len);
	int Decompress(u8* dst, const u8* src, int srclen);

private:
	int getbits(const u8*& src, int& bit_data, int& bit_rest, int bits);
	void dct(const double* src, double* dst0, double* dst1);
};
