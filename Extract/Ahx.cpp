/*
	Copyright (C) 2006
	http://pie.bbspink.com/test/read.cgi/leaf/1141063964/191
	http://pie.bbspink.com/test/read.cgi/leaf/1141063964/258

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

// mpeg2 audio layer-2のデコード方法はmpg123を参考にしました
// http://www.mpg123.de/

#include "StdAfx.h"
#include "Extract/Ahx.h"

#include "ArcFile.h"
#include "Common.h"
#include "Sound/Wav.h"
#include "Utils/BitUtils.h"

#include <array>

#define M_PI    3.14159265358979323846

namespace
{
constexpr std::array<int, 32> bit_alloc_table{ 4,4,4,4,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
constexpr std::array<std::array<int, 16>, 5> offset_table{{
	{ 0 },
	{ 0 },
	{ 0, 1, 3, 4,                                      },
	{ 0, 1, 3, 4, 5, 6, 7, 8,                          },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 }
}};

struct quantclass {
	int nlevels;
	int bits;
};

constexpr std::array<quantclass, 17> qc_table{{
	{     3,  -5},
	{     5,  -7},
	{     7,   3},
	{     9, -10},
	{    15,   4},
	{    31,   5},
	{    63,   6},
	{   127,   7},
	{   255,   8},
	{   511,   9},
	{  1023,  10},
	{  2047,  11},
	{  4095,  12},
	{  8191,  13},
	{ 16383,  14},
	{ 32767,  15},
	{ 65535,  16}
}};

constexpr std::array<int, 257> intwinbase{
     0,    -1,    -1,    -1,    -1,    -1,    -1,    -2,    -2,    -2,    -2,    -3,    -3,    -4,    -4,    -5,
    -5,    -6,    -7,    -7,    -8,    -9,   -10,   -11,   -13,   -14,   -16,   -17,   -19,   -21,   -24,   -26,
   -29,   -31,   -35,   -38,   -41,   -45,   -49,   -53,   -58,   -63,   -68,   -73,   -79,   -85,   -91,   -97,
  -104,  -111,  -117,  -125,  -132,  -139,  -147,  -154,  -161,  -169,  -176,  -183,  -190,  -196,  -202,  -208,
  -213,  -218,  -222,  -225,  -227,  -228,  -228,  -227,  -224,  -221,  -215,  -208,  -200,  -189,  -177,  -163,
  -146,  -127,  -106,   -83,   -57,   -29,     2,    36,    72,   111,   153,   197,   244,   294,   347,   401,
   459,   519,   581,   645,   711,   779,   848,   919,   991,  1064,  1137,  1210,  1283,  1356,  1428,  1498,
  1567,  1634,  1698,  1759,  1817,  1870,  1919,  1962,  2001,  2032,  2057,  2075,  2085,  2087,  2080,  2063,
  2037,  2000,  1952,  1893,  1822,  1739,  1644,  1535,  1414,  1280,  1131,   970,   794,   605,   402,   185,
   -45,  -288,  -545,  -814, -1095, -1388, -1692, -2006, -2330, -2663, -3004, -3351, -3705, -4063, -4425, -4788,
 -5153, -5517, -5879, -6237, -6589, -6935, -7271, -7597, -7910, -8209, -8491, -8755, -8998, -9219, -9416, -9585,
 -9727, -9838, -9916, -9959, -9966, -9935, -9863, -9750, -9592, -9389, -9139, -8840, -8492, -8092, -7640, -7134,
 -6574, -5959, -5288, -4561, -3776, -2935, -2037, -1082,   -70,   998,  2122,  3300,  4533,  5818,  7154,  8540,
  9975, 11455, 12980, 14548, 16155, 17799, 19478, 21189, 22929, 24694, 26482, 28289, 30112, 31947, 33791, 35640,
 37489, 39336, 41176, 43006, 44821, 46617, 48390, 50137, 51853, 53534, 55178, 56778, 58333, 59838, 61289, 62684,
 64019, 65290, 66494, 67629, 68692, 69679, 70590, 71420, 72169, 72835, 73415, 73908, 74313, 74630, 74856, 74992,
 75038
};

std::array<std::array<double, 16>, 5> costable;
std::array<double, 512 + 32> decwin;

int getbits(const u8*& src, int& bit_data, int& bit_rest, int bits)
{
	while (bit_rest < 24)
	{
		bit_data <<= 8;
		bit_data |= *src++;
		bit_rest += 8;
	}

	const int ret = (bit_data >> (bit_rest - bits)) & ((1 << bits) - 1);
	bit_rest -= bits;

	return ret;
}

void dct(const double* src, double* dst0, double* dst1)
{
	double tmp[2][32];

	for (int i = 0; i < 32; i++)
	{
		if (i & 16)
			tmp[0][i] = (-src[i] + src[31 ^ i]) * costable[0][~i & 15];
		else
			tmp[0][i] = (+src[i] + src[31 ^ i]);
	}

	for (int i = 0; i < 32; i++)
	{
		if (i & 8)
			tmp[1][i] = (-tmp[0][i] + tmp[0][15 ^ i]) * costable[1][~i & 7] * ((i & 16) ? -1.0 : 1.0);
		else
			tmp[1][i] = (+tmp[0][i] + tmp[0][15 ^ i]);
	}

	for (int i = 0; i < 32; i++)
	{
		if (i & 4)
			tmp[0][i] = (-tmp[1][i] + tmp[1][7 ^ i]) * costable[2][~i & 3] * ((i & 8) ? -1.0 : 1.0);
		else
			tmp[0][i] = (+tmp[1][i] + tmp[1][7 ^ i]);
	}

	for (int i = 0; i < 32; i++)
	{
		if (i & 2)
			tmp[1][i] = (-tmp[0][i] + tmp[0][3 ^ i]) * costable[3][~i & 1] * ((i & 4) ? -1.0 : 1.0);
		else
			tmp[1][i] = (+tmp[0][i] + tmp[0][3 ^ i]);
	}

	for (int i = 0; i < 32; i++)
	{
		if (i & 1)
			tmp[0][i] = (-tmp[1][i] + tmp[1][1 ^ i]) * costable[4][0] * ((i & 2) ? -1.0 : 1.0);
		else
			tmp[0][i] = (+tmp[1][i] + tmp[1][1 ^ i]);
	}

	//for (int i = 0; i < 8; i++)
		//tmp[0][i * 4 + 2] += tmp[0][i * 4 + 3];
	for (int i = 0; i < 32; i += 4)
	{
		tmp[0][i + 2] += tmp[0][i + 3];
	}

	//for (int i = 0; i < 4; i++)
	//{
	//    tmp[0][i * 8 + 4] += tmp[0][i * 8 + 6];
	//    tmp[0][i * 8 + 6] += tmp[0][i * 8 + 5];
	//    tmp[0][i * 8 + 5] += tmp[0][i * 8 + 7];
	//}
	for (int i = 0; i < 32; i += 8)
	{
		tmp[0][i + 4] += tmp[0][i + 6];
		tmp[0][i + 6] += tmp[0][i + 5];
		tmp[0][i + 5] += tmp[0][i + 7];
	}

	//for (int i = 0; i < 2; i++)
	//{
	//    tmp[0][i * 16 +  8] += tmp[0][i * 16 + 12];
	//    tmp[0][i * 16 + 12] += tmp[0][i * 16 + 10];
	//    tmp[0][i * 16 + 10] += tmp[0][i * 16 + 14];
	//    tmp[0][i * 16 + 14] += tmp[0][i * 16 +  9];
	//    tmp[0][i * 16 +  9] += tmp[0][i * 16 + 13];
	//    tmp[0][i * 16 + 13] += tmp[0][i * 16 + 11];
	//    tmp[0][i * 16 + 11] += tmp[0][i * 16 + 15];
	//}
	for (int i = 0; i < 32; i += 16)
	{
		tmp[0][i +  8] += tmp[0][i + 12];
		tmp[0][i + 12] += tmp[0][i + 10];
		tmp[0][i + 10] += tmp[0][i + 14];
		tmp[0][i + 14] += tmp[0][i +  9];
		tmp[0][i +  9] += tmp[0][i + 13];
		tmp[0][i + 13] += tmp[0][i + 11];
		tmp[0][i + 11] += tmp[0][i + 15];
	}

	dst0[16] = tmp[0][0];
	dst0[15] = tmp[0][16 +  0] + tmp[0][16 +  8];
	dst0[14] = tmp[0][8];
	dst0[13] = tmp[0][16 +  8] + tmp[0][16 +  4];
	dst0[12] = tmp[0][4];
	dst0[11] = tmp[0][16 +  4] + tmp[0][16 + 12];
	dst0[10] = tmp[0][12];
	dst0[ 9] = tmp[0][16 + 12] + tmp[0][16 +  2];
	dst0[ 8] = tmp[0][2];
	dst0[ 7] = tmp[0][16 +  2] + tmp[0][16 + 10];
	dst0[ 6] = tmp[0][10];
	dst0[ 5] = tmp[0][16 + 10] + tmp[0][16 +  6];
	dst0[ 4] = tmp[0][6];
	dst0[ 3] = tmp[0][16 +  6] + tmp[0][16 + 14];
	dst0[ 2] = tmp[0][14];
	dst0[ 1] = tmp[0][16 + 14] + tmp[0][16 +  1];
	dst0[ 0] = tmp[0][1];

	dst1[ 0] = tmp[0][1];
	dst1[ 1] = tmp[0][16 +  1] + tmp[0][16 +  9];
	dst1[ 2] = tmp[0][9];
	dst1[ 3] = tmp[0][16 +  9] + tmp[0][16 +  5];
	dst1[ 4] = tmp[0][5];
	dst1[ 5] = tmp[0][16 +  5] + tmp[0][16 + 13];
	dst1[ 6] = tmp[0][13];
	dst1[ 7] = tmp[0][16 + 13] + tmp[0][16 +  3];
	dst1[ 8] = tmp[0][3];
	dst1[ 9] = tmp[0][16 +  3] + tmp[0][16 + 11];
	dst1[10] = tmp[0][11];
	dst1[11] = tmp[0][16 + 11] + tmp[0][16 +  7];
	dst1[12] = tmp[0][7];
	dst1[13] = tmp[0][16 +  7] + tmp[0][16 + 15];
	dst1[14] = tmp[0][15];
	dst1[15] = tmp[0][16 + 15];
}
} // Anonymous namespace

bool CAhx::Mount(CArcFile* archive)
{
	if (lstrcmpi(archive->GetArcExten(), _T(".ahx")) != 0)
		return false;

	return archive->Mount();
}

bool CAhx::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("AHX"))
		return false;

	// Read AHX
	std::vector<u8> ahx_buf(file_info->size_cmp);
	archive->Read(ahx_buf.data(), ahx_buf.size());

	// Output to convert WAV to AHX
	Decode(archive, ahx_buf.data(), ahx_buf.size());

	return true;
}

void CAhx::Decode(CArcFile* archive, const u8* ahx_buf, size_t ahx_buf_len)
{
	// Convert AHX to WAV
	u32 wav_buf_len = BitUtils::Swap32(*(const u32*)&ahx_buf[12]) * 2;
	std::vector<u8> wav_buf(wav_buf_len + 1152 * 16); // Advance //+ 1152 * 2); // margen = layer-2 frame size
	wav_buf_len = Decompress(wav_buf.data(), ahx_buf, ahx_buf_len);

	// Output
	CWav wav;
	wav.Init(archive, wav_buf_len, BitUtils::Swap32(*(const u32*)&ahx_buf[8]), ahx_buf[7], 16);
	wav.Write(wav_buf.data());
}

int CAhx::Decompress(u8* dst, const u8* src, int srclen)
{
	double powtable[64];
	for (int i = 0; i < 64; i++)
	{
		powtable[i] = pow(2.0, (3 - i) / 3.0);
	}

	double sbsamples[36][32] = {};

	//for (int i = 0; i < 16; i++)
	//	costable[0][i] = 0.5 / cos(M_PI * ((double) i * 2.0 + 1.0) / 64.0);

	//for (int i = 0; i < 8; i++)
	//	costable[1][i] = 0.5 / cos(M_PI * ((double) i * 2.0 + 1.0) / 32.0);

	//for (int i = 0; i < 4; i++)
	//	costable[2][i] = 0.5 / cos(M_PI * ((double) i * 2.0 + 1.0) / 16.0);

	//for (int i = 0; i < 2; i++)
	//	costable[3][i] = 0.5 / cos(M_PI * ((double) i * 2.0 + 1.0) /  8.0);

	//for (int i = 0; i < 1; i++)
	//	costable[4][i] = 0.5 / cos(M_PI * ((double) i * 2.0 + 1.0) /  4.0);

	for (int i = 0; i < 16; i++)
		costable[0][i] = 0.5 / cos(M_PI * ((i << 1) + 1) / 64.0);

	for (int i = 0; i < 8; i++)
		costable[1][i] = 0.5 / cos(M_PI * ((i << 1) + 1) / 32.0);

	for (int i = 0; i < 4; i++)
		costable[2][i] = 0.5 / cos(M_PI * ((i << 1) + 1) / 16.0);

	for (int i = 0; i < 2; i++)
		costable[3][i] = 0.5 / cos(M_PI * ((i << 1) + 1) /  8.0);

	for (int i = 0; i < 1; i++)
		costable[4][i] = 0.5 / cos(M_PI * ((i << 1) + 1) /  4.0);

	for (int i = 0, j = 0; i < 256; i++, j += 32)
	{
		if (j < 512 + 16)
			decwin[j] = decwin[j + 16] = intwinbase[i] / 65536.0 * 32768.0 * ((i & 64) ? +1.0 : -1.0);

		if ((i & 31) == 31)
			j -= 1023;
	}

	for (int i = 0, j = 8; i < 256; i++, j += 32)
	{
		if (j < 512 + 16)
			decwin[j] = decwin[j + 16] = intwinbase[256 - i] / 65536.0 * 32768.0 * ((i & 64) ? +1.0 : -1.0);

		if ((i & 31) == 31)
			j -= 1023;
	}

	src += src[2] * 256 + src[3] + 4;
	int phase = 0;
	int bit_alloc[32];
	int scfsi[32];
	int scalefactor[32][3];
	double dctbuf[2][16][17];
	short* dst_p = (short*)dst;
	int bit_rest = 0, bit_data;
	const u8* src_start = src;
	int frame = 0;

	while (src - src_start < srclen && getbits(src, bit_data, bit_rest, 12) == 0xfff)
	{
		frame++;
		getbits(src, bit_data, bit_rest, 1); // LSF
		getbits(src, bit_data, bit_rest, 2); // layer
		getbits(src, bit_data, bit_rest, 1); // CRC
		getbits(src, bit_data, bit_rest, 4); // bitrate
		getbits(src, bit_data, bit_rest, 2); // freq
		getbits(src, bit_data, bit_rest, 1); // padding
		getbits(src, bit_data, bit_rest, 1); // gap
		getbits(src, bit_data, bit_rest, 2); // mode
		getbits(src, bit_data, bit_rest, 2); // mode_ext
		getbits(src, bit_data, bit_rest, 1); // protect
		getbits(src, bit_data, bit_rest, 1); // copy
		getbits(src, bit_data, bit_rest, 2); // emphasis

		for (int sb = 0; sb < 30; sb++)
		{
			bit_alloc[sb] = getbits(src, bit_data, bit_rest, bit_alloc_table[sb]);
		}

		for (int sb = 0; sb < 30; sb++)
		{
			if (bit_alloc[sb])
				scfsi[sb] = getbits(src, bit_data, bit_rest, 2);
		}

		for (int sb = 0; sb < 30; sb++)
		{
			if (bit_alloc[sb])
			{
				scalefactor[sb][0] = getbits(src, bit_data, bit_rest, 6);
				switch (scfsi[sb])
				{
					case 0:
						scalefactor[sb][1] = getbits(src, bit_data, bit_rest, 6);
						scalefactor[sb][2] = getbits(src, bit_data, bit_rest, 6);
						break;
					case 1:
						scalefactor[sb][1] = scalefactor[sb][0];
						scalefactor[sb][2] = getbits(src, bit_data, bit_rest, 6);
						break;
					case 2:
						scalefactor[sb][1] = scalefactor[sb][0];
						scalefactor[sb][2] = scalefactor[sb][0];
						break;
					case 3:
						scalefactor[sb][1] = scalefactor[sb][2] = getbits(src, bit_data, bit_rest, 6);
						break;
				}
			}
		}

		for (int gr = 0; gr < 12; gr++)
		{
			for (int sb = 0; sb < 30; sb++)
			{
				if (bit_alloc[sb]) {
					int index = offset_table[bit_alloc_table[sb]][bit_alloc[sb] - 1];
					int q;

					if (qc_table[index].bits < 0)
					{
						int t = getbits(src, bit_data, bit_rest, -qc_table[index].bits);
						q = (t % qc_table[index].nlevels) * 2 -qc_table[index].nlevels + 1;
						sbsamples[gr * 3 + 0][sb] = (double) q / (double)qc_table[index].nlevels;

						t /= qc_table[index].nlevels;
						q = (t % qc_table[index].nlevels) * 2 -qc_table[index].nlevels + 1;
						sbsamples[gr * 3 + 1][sb] = (double) q / (double)qc_table[index].nlevels;

						t /= qc_table[index].nlevels;
						q = t * 2 -qc_table[index].nlevels + 1;
						sbsamples[gr * 3 + 2][sb] = (double) q / (double)qc_table[index].nlevels;
					}

					else
					{
						q = getbits(src, bit_data, bit_rest, qc_table[index].bits) * 2 - qc_table[index].nlevels + 1;
						sbsamples[gr * 3 + 0][sb] = (double) q / (double)qc_table[index].nlevels;

						q = getbits(src, bit_data, bit_rest, qc_table[index].bits) * 2 - qc_table[index].nlevels + 1;
						sbsamples[gr * 3 + 1][sb] = (double) q / (double) qc_table[index].nlevels;

						q = getbits(src, bit_data, bit_rest, qc_table[index].bits) * 2 - qc_table[index].nlevels + 1;
						sbsamples[gr * 3 + 2][sb] = (double) q / (double)qc_table[index].nlevels;
					}
				}

				else
				{
					sbsamples[gr * 3 + 0][sb] = 0;
					sbsamples[gr * 3 + 1][sb] = 0;
					sbsamples[gr * 3 + 2][sb] = 0;
				}

				//sbsamples[gr * 3 + 0][sb] *= powtable[scalefactor[sb][gr / 4]];
				//sbsamples[gr * 3 + 1][sb] *= powtable[scalefactor[sb][gr / 4]];
				//sbsamples[gr * 3 + 2][sb] *= powtable[scalefactor[sb][gr / 4]];
				sbsamples[gr * 3 + 0][sb] *= powtable[scalefactor[sb][gr >> 2]];
				sbsamples[gr * 3 + 1][sb] *= powtable[scalefactor[sb][gr >> 2]];
				sbsamples[gr * 3 + 2][sb] *= powtable[scalefactor[sb][gr >> 2]];
			}
		}

		// synth
		for (int gr = 0; gr < 36; gr++)
		{
			if (phase & 1)
				dct(sbsamples[gr], dctbuf[0][(phase + 1) & 15], dctbuf[1][phase]);
			else
				dct(sbsamples[gr], dctbuf[1][phase], dctbuf[0][phase + 1]);

			double sum;
			double* win = decwin.data() + 16 - (phase | 1);

			for (int i = 0; i < 16; i++)
			{
				sum  = *win++ * dctbuf[phase&1][0][i];
				sum -= *win++ * dctbuf[phase&1][1][i];
				sum += *win++ * dctbuf[phase&1][2][i];
				sum -= *win++ * dctbuf[phase&1][3][i];
				sum += *win++ * dctbuf[phase&1][4][i];
				sum -= *win++ * dctbuf[phase&1][5][i];
				sum += *win++ * dctbuf[phase&1][6][i];
				sum -= *win++ * dctbuf[phase&1][7][i];
				sum += *win++ * dctbuf[phase&1][8][i];
				sum -= *win++ * dctbuf[phase&1][9][i];
				sum += *win++ * dctbuf[phase&1][10][i];
				sum -= *win++ * dctbuf[phase&1][11][i];
				sum += *win++ * dctbuf[phase&1][12][i];
				sum -= *win++ * dctbuf[phase&1][13][i];
				sum += *win++ * dctbuf[phase&1][14][i];
				sum -= *win++ * dctbuf[phase&1][15][i];

				if (sum >= 32767)
					*dst_p++ = 32767;
				else if (sum <= -32767)
					*dst_p++ = -32767;
				else
					*dst_p++ = static_cast<short>(sum);

				win += 16;
			}

			sum  = win[0] * dctbuf[phase & 1][0][16];
			sum += win[2] * dctbuf[phase & 1][2][16];
			sum += win[4] * dctbuf[phase & 1][4][16];
			sum += win[6] * dctbuf[phase & 1][6][16];
			sum += win[8] * dctbuf[phase & 1][8][16];
			sum += win[10] * dctbuf[phase & 1][10][16];
			sum += win[12] * dctbuf[phase & 1][12][16];
			sum += win[14] * dctbuf[phase & 1][14][16];

			if (sum >= 32767)
				*dst_p++ = 32767;
			else if (sum <= -32767)
				*dst_p++ = -32767;
			else
				*dst_p++ = static_cast<short>(sum);

			win += -16 + (phase | 1) * 2;

			for (int i = 15; i >= 1; i--)
			{
				sum  = -*--win * dctbuf[phase&1][0][i];
				sum -= *--win * dctbuf[phase&1][1][i];
				sum -= *--win * dctbuf[phase&1][2][i];
				sum -= *--win * dctbuf[phase&1][3][i];
				sum -= *--win * dctbuf[phase&1][4][i];
				sum -= *--win * dctbuf[phase&1][5][i];
				sum -= *--win * dctbuf[phase&1][6][i];
				sum -= *--win * dctbuf[phase&1][7][i];
				sum -= *--win * dctbuf[phase&1][8][i];
				sum -= *--win * dctbuf[phase&1][9][i];
				sum -= *--win * dctbuf[phase&1][10][i];
				sum -= *--win * dctbuf[phase&1][11][i];
				sum -= *--win * dctbuf[phase&1][12][i];
				sum -= *--win * dctbuf[phase&1][13][i];
				sum -= *--win * dctbuf[phase&1][14][i];
				sum -= *--win * dctbuf[phase&1][15][i];

				if (sum >= 32767)
					*dst_p++ = 32767;
				else if (sum <= -32767)
					*dst_p++ = -32767;
				else
					*dst_p++ = static_cast<short>(sum);

				win -= 16;
			}

			phase = phase - 1 & 15;
		}

		// skip padding bits
		if (bit_rest & 7)
			getbits(src, bit_data, bit_rest, bit_rest & 7);
	}

	return (int) ((char*)dst_p - (char*)dst);
}
