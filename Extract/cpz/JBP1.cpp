#include "StdAfx.h"
#include "../../ExtractBase.h"
#include "JBP1.h"

/// Decompression
void CJBP1::Decomp(u8* pbtDst, const u8* pbtSrc, u16 wBpp, const u8* pbtAlpha, u32 dwAlphaSize)
{
	static const u8 original_order[64] =
	{
		 1,  8, 16,  9,  2,  3, 10, 17,
		24, 32, 25, 18, 11,  4,  5, 12,
		19, 26, 33, 40, 48, 41, 34, 27,
		20, 13,  6,  7, 14, 21, 28, 35,
		42, 49, 56, 57, 50, 43, 36, 29,
		22, 15, 23, 30, 37, 44, 51, 58,
		59, 52, 45, 38, 31, 39, 46, 53,
		60, 61, 54, 47, 55, 62, 63,  0
	};

	const u8* lpin = pbtSrc;
	const s32 width = *(const u16*)&lpin[0x10];
	const s32 height = *(const u16*)&lpin[0x12];

	if (wBpp == 0)
	{
		wBpp = *(const u16*)&lpin[0x14];
	}

	const s32 colors = wBpp >> 3;

	s32 ww, hh;
	const u32 type = (*(const u32*)&lpin[8] >> 28) & 3;

	switch (type)
	{
	case 0:
		ww = (width + 0x07) & 0xFFFFFFF8;
		hh = (height + 0x07) & 0xFFFFFFF8;
		break;

	case 1:
		ww = (width + 0x0F) & 0xFFFFFFF0;
		hh = (height + 0x0F) & 0xFFFFFFF0;
		break;

	case 2:
		ww = (width + 0x1F) & 0xFFFFFFE0;
		hh = (height + 0x0F) & 0xFFFFFFF0;
		break;

	default:
		return;
	}

	// Frequency Tables : 16 elements

	lpin += *(const u32*) &lpin[4];

	u32 freq_dc[128];
	u32 freq_ac[128];

	memcpy(freq_dc, lpin, 0x40);
	lpin += 0x40;

	memcpy(freq_ac, lpin, 0x40);
	lpin += 0x40;

	// Run length of zero
	// Free beginning portion of the huffman table
	// Table is not rewritten due to problems that occur

	u8 huff_tbl[0x1508];

	for (size_t i = 0; i < 16; i++)
	{
		huff_tbl[i] = *lpin++ + 1;
	}

	u16 quant_y[0x40];
	u16 quant_c[0x40];
	if (*(const u32*)&pbtSrc[8] & 0x08000000)
	{
		for (size_t i = 0; i < 0x40; i++)
			quant_y[i] = *lpin++;

		for (size_t i = 0; i < 0x40; i++)
			quant_c[i] = *lpin++;
	}

	const u8* lpin2 = lpin + *(const u32*)&pbtSrc[0x1C];

	const u32 colorSize = ww * hh;
	std::vector<u8> dst2(colorSize * 4);
	u8* lpout = dst2.data();

	const s32 x_size = ww >> 4;
	const s32 y_size = hh >> 4;

	const u32 tmpSize = x_size * y_size * 3 * 8;
	std::vector<u8> tmp(tmpSize);
	u8* pTmp = tmp.data();

	u32 bit_buffer = 0;
	u32 bit_remain = 0;
	u32 EOT = MakeTree(huff_tbl, 16, freq_dc); //End Of Tree == Number of Datas
	const s32 size = x_size * y_size * 3 * 2;

	// Binary tree search of the 16 elements (Decompressed DC)

	for (s32 i = 0; i < size; i++)
	{
		u32 code = EOT - 1;
		while (code >= 16)
		{
			if (bit_remain == 0)
			{
				bit_buffer = *(const u32*)lpin;
				bit_remain = 0x20;
				lpin += 4;
			}
			code = *(const u32*)&huff_tbl[0x100 + (((bit_buffer & 1) << 9) + code) * 4];
			bit_remain--;
			bit_buffer >>= 1;
		}

		u32 x = (code > 0) ? GetNBit(lpin, code, bit_buffer, bit_remain) : 0;
		if (x < (1 << (code - 1)))
			x = x - (1 << code) + 1;

		*(u32*)&tmp[i * 4] = x;
		if (i != 0)
			*(u32*)&tmp[i * 4] += *(const u32*)&tmp[i * 4 - 4];
	}

	u8 dct_tbl[0x300];
	bit_buffer = 0;
	bit_remain = 0;
	EOT = MakeTree(huff_tbl, 16, freq_ac);

	for (s32 y = 0; y < y_size; y++)
	{
		u8* lpDst1 = lpout + 0x20;
		u8* lpDst2 = lpout + ww * 4 * 9;

		for (s32 x = 0; x < x_size; x++)
		{
			memset(dct_tbl, 0, 0x300);
			u16* f = (u16*)dct_tbl;

			for (s32 n = 0; n < 6; n++)
			{
				f[0] = *(const u16*)pTmp;
				pTmp += 4;

				// Huffman tree search to element 63 (Decompress AC)
				// Close to a JPEG huffman variant
				for (s32 i = 0; i < 63;)
				{
					u32 code = EOT - 1;
					while (code >= 16)
					{
						if (bit_remain == 0)
						{
							bit_buffer = *(const u32*)lpin2;
							bit_remain = 0x20;
							lpin2 += 4;
						}
						code = *(const u32*)&huff_tbl[0x100 + (((bit_buffer & 1) << 9) + code) * 4];
						bit_remain--;
						bit_buffer >>= 1;
					}

					if (code == 15)
					{
						break;
					}

					if (code == 0)
					{
						const u8* p = huff_tbl;
						while (true)
						{
							if (bit_remain == 0)
							{
								bit_buffer = *(const u32*)lpin2;
								bit_remain = 0x20;
								lpin2 += 4;
							}

							const u32 bit = bit_buffer & 1;
							bit_remain--;
							bit_buffer >>= 1;

							if (bit == 0)
							{
								break;
							}

							p++;
						}
						i += *p;
					}
					else
					{
						u32 bit = GetNBit(lpin2, code, bit_buffer, bit_remain);
						if (bit < (1 << (code - 1)))
							bit = bit - (1 << code) + 1;
						f[original_order[i]] = bit;
						i++;
					}
				}

				f += 64;
			}

			DCT(dct_tbl + 0 * 2, (u8*)quant_y);
			DCT(dct_tbl + 64 * 2, (u8*)quant_y);
			DCT(dct_tbl + 128 * 2, (u8*)quant_y);
			DCT(dct_tbl + 192 * 2, (u8*)quant_y);
			DCT(dct_tbl + 256 * 2, (u8*)quant_c);
			DCT(dct_tbl + 320 * 2, (u8*)quant_c);

			// 2 line by line processing
			// ¡ 
			//   
			u8* dc = lpDst1 - 0x20;
			u8* ac = lpDst1 - 0x20 + ww * 4;
			YCC2RGB(dc, ac, (const s16*)dct_tbl, (const s16*)(dct_tbl + 320 * 2), ww * 4);

			//  ¡
			//   
			dc = lpDst1;
			ac = lpDst2 + 0x20 - ww * 4 * 8;
			YCC2RGB(dc, ac, (const s16*)(dct_tbl + 64 * 2), (const s16*)(dct_tbl + 324 * 2), ww * 4);

			//   
			// ¡ 
			dc = lpDst1 + ((ww * 4) << 3) - 0x20;
			ac = lpDst2;
			YCC2RGB(dc, ac, (const s16*)(dct_tbl + 128 * 2), (const s16*)(dct_tbl + 352 * 2), ww * 4);

			//   
			//  ¡
			dc = lpDst2 + 0x20 - ww * 4;
			ac = lpDst2 + 0x20;
			YCC2RGB(dc, ac, (const s16*)(dct_tbl + 192 * 2), (const s16*)(dct_tbl + 356 * 2), ww * 4);

			lpDst1 += 0x40;
			lpDst2 += 0x40;

		} //x_size

		lpout += (ww << 6);
	} //y_size

	const s32 lwidth = width * 4;
	const s32 lww = ww * 4;
	u8* pDst = &pbtDst[0];
	u8* pDst2 = &dst2[0];

	// Cut and copy extra data
	for (s32 y = 0; y < height; y++)
	{
		//for (int x = 0; x < lwidth; x++)
		for (s32 x = 0; x < width; x++)
		{
			memcpy(pDst, pDst2, colors);
			pDst += colors;
			pDst2 += 4;
			//*pDst++ = *pDst2++;
		}
		pDst2 += lww - lwidth;
	}

	pDst = &pbtDst[3];
	// Put a value for the alpha
	if (wBpp == 32)
	{
		const u8* alpha_end = pbtAlpha + dwAlphaSize;
		while (pbtAlpha < alpha_end)
		{
			if (*pbtAlpha == 0 || *pbtAlpha == 0xFF)
			{
				for (u8 x = 0; x < pbtAlpha[1]; x++)
				{
					*pDst = pbtAlpha[0];
					pDst += 4;
				}
				pbtAlpha += 2;
			}
			else
			{
				*pDst = *pbtAlpha++;
				pDst += 4;
			}
		}
	}
	//else
	//{
	//  colorSize = width * height;
	//  for (int i = 0; i < (int)colorSize; i++)
	//  {
	//     *pDst = 0xFF;
	//     pDst += 4;
	//  }
	//}
}

void CJBP1::DCT(u8* arg1, u8* arg2)
{
	s32 a, b, c, d;
	s32 w, x, y, z;
	s32 s, t, u, v, n;

	u8* lp1 = arg1;
	u8* lp2 = arg2;
	s32 i = 8;

	do
	{
		if (*(s16*)&lp1[0x10] == 0 &&
		    *(s16*)&lp1[0x20] == 0 &&
		    *(s16*)&lp1[0x30] == 0 &&
		    *(s16*)&lp1[0x40] == 0 &&
		    *(s16*)&lp1[0x50] == 0 &&
		    *(s16*)&lp1[0x60] == 0 &&
		    *(s16*)&lp1[0x70] == 0)
		{
			*(s16*)&lp1[0x00] =
			*(s16*)&lp1[0x10] =
			*(s16*)&lp1[0x20] =
			*(s16*)&lp1[0x30] =
			*(s16*)&lp1[0x40] =
			*(s16*)&lp1[0x50] =
			*(s16*)&lp1[0x60] =
			*(s16*)&lp1[0x70] = *(s16*)&lp1[0] * *(s16*)&lp2[0];
		}
		else
		{
			c = *(s16*)&lp2[0x20] * *(s16*)&lp1[0x20];
			d = *(s16*)&lp2[0x60] * *(s16*)&lp1[0x60];
			x = c + d;
			x = (x * 0x00008A8B) >> 16;
			c = ((c * 0x0000C3EF) >> 16) + x; //sar==unsigned‚¾‚¯‚Ç
			d = ((d * (s32)0xFFFE26FA) >> 16) + x;
			a = *(s16*)&lp1[0x00] * *(s16*)&lp2[0x00];
			b = *(s16*)&lp1[0x40] * *(s16*)&lp2[0x40];
			w = a + b + c; //[esp+20]
			x = a + b - c; //[esp+2C]
			y = a - b + d; //[esp+24]
			z = a - b - d; //[esp+28]

			c = *(s16*)&lp1[0x70] * *(s16*)&lp2[0x70];
			d = *(s16*)&lp1[0x50] * *(s16*)&lp2[0x50];
			a = *(s16*)&lp1[0x30] * *(s16*)&lp2[0x30];
			b = *(s16*)&lp1[0x10] * *(s16*)&lp2[0x10];
			n = ((a + b + c + d) * 0x00012D06) >> 16;

			u = n + ((c * 0x00004C73) >> 16)
			      + (((c + a) * (s32)0xFFFE09D7) >> 16)  //[esp+18]
			      + (((c + b) * (s32)0xFFFF199C) >> 16); //[esp+14]

			v = n + ((d * 0x00020D99) >> 16)
			      + (((d + b) * (s32)0xFFFF9C1E) >> 16)  //[esp+1C]
			      + (((d + a) * (s32)0xFFFD6FE5) >> 16); //[esp+10]

			t = n + ((b * 0x00018056) >> 16)
			      + (((d + b) * (s32)0xFFFF9C1E) >> 16)  //[esp+1C]
			      + (((c + b) * (s32)0xFFFF199C) >> 16); //[esp+14]

			s = n + ((a * 0x0003129D) >> 16)
			      + (((c + a) * (s32)0xFFFE09D7) >> 16)  //[esp+18]
			      + (((d + a) * (s32)0xFFFD6FE5) >> 16); //[esp+10]

			*(s16*)&lp1[0x00] = static_cast<s16>(w + t);
			*(s16*)&lp1[0x70] = static_cast<s16>(w - t);
			*(s16*)&lp1[0x10] = static_cast<s16>(y + s);
			*(s16*)&lp1[0x60] = static_cast<s16>(y - s);
			*(s16*)&lp1[0x20] = static_cast<s16>(z + v);
			*(s16*)&lp1[0x50] = static_cast<s16>(z - v);
			*(s16*)&lp1[0x30] = static_cast<s16>(x + u);
			*(s16*)&lp1[0x40] = static_cast<s16>(x - u);
		}

		lp1 += 2;
		lp2 += 2;
		i--;
	} while (i > 0);

	lp1 = arg1;
	i = 8; //[esp+3C] == arg2???

	do
	{
		c = *(s16*)&lp1[4];
		d = *(s16*)&lp1[12];
		b = *(s16*)&lp1[8];
		x = c + d;
		x = ((x * 0x00008A8B) >> 16);
		c = ((c * 0x0000C3EF) >> 16) + x;
		d = ((d * (s32)0xFFFE26FA) >> 16) + x;
		a = *(s16*)&lp1[0];
		w = a + b + c; //[esp+20]
		x = a + b - c; //[esp+2C]
		y = a - b + d; //[esp+24]
		z = a - b - d; //[esp+28]

		d = *(s16*)&lp1[10];
		b = *(s16*)&lp1[2];
		c = *(s16*)&lp1[14];
		a = *(s16*)&lp1[6];
		n = (((a + b + c + d) * 0x00012D06) >> 16);

		s = n + ((a * 0x0003129D) >> 16)
		      + (((a + c) * (s32)0xFFFE09D7) >> 16)  //[esp+18]
		      + (((a + d) * (s32)0xFFFD6FE5) >> 16); //[esp+10]

		t = n + ((b * 0x00018056) >> 16)
		      + (((b + d) * (s32)0xFFFF9C1E) >> 16)
		      + (((b + c) * (s32)0xFFFF199C) >> 16); //[esp+14]

		u = n + ((c * 0x00004C73) >> 16)
		      + (((b + c) * (s32)0xFFFF199C) >> 16)  //[esp+14]
		      + (((a + c) * (s32)0xFFFE09D7) >> 16); //[esp+18]

		v = n + ((d * 0x00020D99) >> 16)
		      + (((b + d) * (s32)0xFFFF9C1E) >> 16)
		      + (((a + d) * (s32)0xFFFD6FE5) >> 16); //[esp+10]

		*(s16*)&lp1[0]  = static_cast<s16>((w + t) >> 3);
		*(s16*)&lp1[14] = static_cast<s16>((w - t) >> 3);
		*(s16*)&lp1[2]  = static_cast<s16>((y + s) >> 3);
		*(s16*)&lp1[12] = static_cast<s16>((y - s) >> 3);
		*(s16*)&lp1[4]  = static_cast<s16>((z + v) >> 3);
		*(s16*)&lp1[10] = static_cast<s16>((z - v) >> 3);
		*(s16*)&lp1[6]  = static_cast<s16>((x + u) >> 3);
		*(s16*)&lp1[8]  = static_cast<s16>((x - u) >> 3);

		lp1 += 16;
		i--;
	} while (i > 0);
}

s32 CJBP1::GetNBit(const u8*& lpin, u32 code, u32& bit_buffer, u32& bit_remain)
{
	s32 i = 0;

	if (code == 0)
	{
		return 0;
	}

	if (code > bit_remain)
	{
		for (; code > 0; code--)
		{
			if (bit_remain == 0)
			{
				bit_buffer = *(const u32*)lpin;
				lpin += 4;

				bit_remain = 0x20;
			}

			i = (bit_buffer & 1) + i * 2;

			bit_remain--;
			bit_buffer >>= 1;
		}
	}
	else
	{
		for (; code > 0; code--)
		{
			i = (bit_buffer & 1) + i * 2;

			bit_remain--;
			bit_buffer >>= 1;
		}
	}

	return i;
}

s32 CJBP1::MakeTree(u8* lp1, s32 size, u32* lp2)
{
	s32 n, x, c, d;

	u8* lp3 = lp1 + 0x900;

	for (s32 i = 0x102; i > 0; i--)
	{
		*(u32*)&lp3[0x800]  = 0;
		*(u32*)&lp3[0]      = 0;
		*(u32*)&lp3[-0x800] = 0;

		lp3 += 4;
	}

	c = ~size + 1;                //neg [esp+10]
	lp3 = lp1 + size * 4 + 0x900; //[esp+14]

	// Get two minimum frequency values

	for (;;)
	{
		d = n = -100; //-0x64;
		x = 0x7D2B74FF;

		for (s32 i = 0; i < size; i++)
		{
			if (lp2[i] < x)
			{
				n = i;
				x = lp2[i];
			}
		}

		x = 0x7D2B74FF;

		for (s32 i = 0; i < size; i++)
		{
			if (i != n && lp2[i] < x)
			{
				d = i;
				x = lp2[i];
			}
		}//440E54

		if ((n < 0) || (d < 0))
		{
			break;
		}

		*(u32*)&lp3[-0x800] = n;
		*(u32*)&lp3[0] = d;
		*(u32*)&lp1[n * 4 + 0x1100] = size;
		*(u32*)&lp1[d * 4 + 0x1100] = c;
		lp2[size] = lp2[n] + lp2[d];
		size++;
		c--;
		lp3 += 4;
		lp2[n] = 0x7D2B7500;
		lp2[d] = 0x7D2B7500;
	}

	return size;
}

/// YCbCr => RGB
void CJBP1::YCC2RGB(u8* dc, u8* ac, const s16* Y, const s16* CbCr, s32 line)
{
	static bool is_tbl = false;
	static u8 fixed_byte_tbl[0x300]; //462C48

	// Faster table truncation, saturation
	// -0x80 is less than 0, 0x80 is less than 0xFF 

	if (!is_tbl)
	{
		for (size_t n = 0; n < 0x100; n++)
			fixed_byte_tbl[n] = 0;

		for (size_t n = 0; n < 0x100; n++)
			fixed_byte_tbl[n + 0x100] = static_cast<u8>(n);

		for (size_t n = 0; n < 0x100; n++)
			fixed_byte_tbl[n + 0x200] = 0xFF;

		is_tbl = true;
	}

	const u8* tbl = fixed_byte_tbl;

	for (s32 n = 4; n > 0; n--)
	{
		s32 r, g, b, c, d, w, x, y, z;

		c = CbCr[0]; //cr
		d = CbCr[-64]; //cb
		r = ((c * 0x166F0) >> 16); //[588]
		g = ((d * 0x5810) >> 16) + ((c * 0xB6C0) >> 16); //[58C]
		b = ((d * 0x1C590) >> 16);
		w = Y[1] + 0x180;
		x = Y[0] + 0x180;
		y = Y[8] + 0x180;
		z = Y[9] + 0x180;
		dc[0]        = tbl[x + b];
		ac[4 - line] = tbl[w + b];
		ac[0]        = tbl[y + b];
		ac[4]        = tbl[z + b];
		ac[1 - line] = tbl[x - g];
		ac[5 - line] = tbl[w - g];
		ac[1]        = tbl[y - g];
		ac[5]        = tbl[z - g];
		ac[2 - line] = tbl[x + r];
		ac[6 - line] = tbl[w + r];
		ac[2]        = tbl[y + r];
		ac[6]        = tbl[z + r];
		Y += 2;
		c = CbCr[1]; //[578]
		d = CbCr[-64 + 1];
		r = ((c * 0x166F0) >> 16);
		g = ((d * 0x5810) >> 16) + ((c * 0xB6C0) >> 16);
		b = ((d * 0x1C590) >> 16);
		w = Y[1] + 0x180;
		x = Y[0] + 0x180;
		y = Y[8] + 0x180;
		z = Y[9] + 0x180;
		ac[8 - line]  = tbl[x + b];
		ac[12 - line] = tbl[w + b];
		ac[8]         = tbl[y + b];
		ac[12]        = tbl[z + b];
		ac[9 - line]  = tbl[x - g];
		ac[13 - line] = tbl[w - g];
		ac[9]         = tbl[y - g];
		ac[13]        = tbl[z - g];
		ac[10 - line] = tbl[x + r];
		ac[14 - line] = tbl[w + r];
		ac[10]        = tbl[y + r];
		ac[14]        = tbl[z + r];
		Y += 2;
		c = CbCr[2];
		d = CbCr[-64 + 2];
		r = ((c * 0x166F0) >> 16);
		g = ((d * 0x5810) >> 16) + ((c * 0xB6C0) >> 16);
		b = ((d * 0x1C590) >> 16);
		w = Y[1] + 0x180;
		x = Y[0] + 0x180;
		y = Y[8] + 0x180;
		z = Y[9] + 0x180;
		ac[16 - line] = tbl[x + b];
		ac[20 - line] = tbl[w + b];
		ac[16]        = tbl[y + b];
		ac[20]        = tbl[z + b];
		ac[17 - line] = tbl[x - g];
		ac[21 - line] = tbl[w - g];
		ac[17]        = tbl[y - g];
		ac[21]        = tbl[z - g];
		ac[18 - line] = tbl[x + r];
		ac[22 - line] = tbl[w + r];
		ac[18]        = tbl[y + r];
		ac[22]        = tbl[z + r];
		Y += 2;
		c = CbCr[3];
		d = CbCr[-64 + 3];
		r = ((c * 0x166F0) >> 16);
		g = ((d * 0x5810) >> 16) + ((c * 0xB6C0) >> 16);
		b = ((d * 0x1C590) >> 16);
		w = Y[1] + 0x180;
		x = Y[0] + 0x180;
		y = Y[8] + 0x180;
		z = Y[9] + 0x180;
		ac[24 - line] = tbl[x + b];
		ac[28 - line] = tbl[w + b];
		ac[24]        = tbl[y + b];
		ac[28]        = tbl[z + b];
		ac[25 - line] = tbl[x - g];
		ac[29 - line] = tbl[w - g];
		ac[25]        = tbl[y - g];
		ac[29]        = tbl[z - g];
		ac[26 - line] = tbl[x + r];
		ac[30 - line] = tbl[w + r];
		ac[26]        = tbl[y + r];
		ac[30]        = tbl[z + r];

		dc += line * 2; //[590]
		ac += line * 2;
		Y += 2 + 8;
		CbCr += 8;
	}
}
