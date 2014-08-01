#include "stdafx.h"
#include "../../ExtractBase.h"
#include "JBP1.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Decompression

void CJBP1::Decomp(BYTE* pbtDst, const BYTE* pbtSrc, WORD wBpp, const BYTE* pbtAlpha, DWORD dwAlphaSize)
{
	static const BYTE original_order[64] =
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

	const BYTE* lpin = pbtSrc;
	long width = *(WORD*)&lpin[0x10];
	long height = *(WORD*)&lpin[0x12];

	if (wBpp == 0)
	{
		wBpp = *(WORD*)&lpin[0x14];
	}

	int colors = wBpp >> 3;

	LONG ww, hh;
	DWORD type = (*(LPDWORD)&lpin[8] >> 28) & 3;

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

	lpin += *(LPDWORD) &lpin[4];

	DWORD freq_dc[128];
	DWORD freq_ac[128];

	memcpy(freq_dc, lpin, 0x40);
	lpin += 0x40;

	memcpy(freq_ac, lpin, 0x40);
	lpin += 0x40;

	// Run length of zero
	// Free beginning portion of the huffman table
	// Table is not rewritten due to problems that occur

	BYTE huff_tbl[0x1508];

	for (int i = 0; i < 16; i++)
	{
		huff_tbl[i] = *lpin++ + 1;
	}

	WORD quant_y[0x40];
	WORD quant_c[0x40];
	if (*(LPDWORD)&pbtSrc[8] & 0x08000000)
	{
		for (int i = 0; i < 0x40; i++)
			quant_y[i] = *lpin++;

		for (int i = 0; i < 0x40; i++)
			quant_c[i] = *lpin++;
	}

	const BYTE* lpin2 = lpin + *(LPDWORD)&pbtSrc[0x1C];

	DWORD colorSize = ww * hh;
	YCMemory<BYTE> dst2(colorSize * 4);
	LPBYTE lpout = &dst2[0];

	int x_size = ww >> 4;
	int y_size = hh >> 4;

	DWORD tmpSize = x_size * y_size * 3 * 8;
	YCMemory<BYTE> tmp(tmpSize);
	LPBYTE pTmp = &tmp[0];

	DWORD bit_buffer = 0;
	DWORD bit_remain = 0;
	DWORD EOT = MakeTree(huff_tbl, 16, freq_dc); //End Of Tree == Number of Datas
	int size = x_size * y_size * 3 * 2;

	// Binary tree search of the 16 elements (Decompressed DC)

	for (int i = 0; i < size; i++)
	{
		DWORD code = EOT - 1;
		while (code >= 16)
		{
			if (bit_remain == 0)
			{
				bit_buffer = *(LPDWORD)lpin; lpin += 4;
				bit_remain = 0x20;
			}
			code = *(LPDWORD)&huff_tbl[0x100 + (((bit_buffer & 1) << 9) + code) * 4];
			bit_remain--;
			bit_buffer >>= 1;
		}

		DWORD x = (code > 0) ? GetNBit(lpin, code, bit_buffer, bit_remain) : 0;
		if (x < (1 << (code - 1)))
			x = x - (1 << code) + 1;

		*(LPDWORD)&tmp[i * 4] = x;
		if (i != 0)
			*(LPDWORD)&tmp[i * 4] += *(LPDWORD)&tmp[i * 4 - 4];
	}

	BYTE dct_tbl[0x300];
	bit_buffer = 0;
	bit_remain = 0;
	EOT = MakeTree(huff_tbl, 16, freq_ac);

	for (int y = 0; y < y_size; y++)
	{
		LPBYTE lpDst1 = lpout + 0x20;
		LPBYTE lpDst2 = lpout + ww * 4 * 9;

		for (int x = 0; x < x_size; x++)
		{
			memset(dct_tbl, 0, 0x300);
			LPWORD f = (LPWORD)dct_tbl;

			for (int n = 0; n < 6; n++)
			{
				f[0] = *(LPWORD)pTmp;
				pTmp += 4;

				// Huffman tree search to element 63 (Decompress AC)
				// Close to a JPEG huffman variant
				for (int i = 0; i < 63;)
				{
					DWORD code = EOT - 1;
					while (code >= 16)
					{
						if (bit_remain == 0)
						{
							bit_buffer = *(LPDWORD)lpin2; lpin2 += 4;
							bit_remain = 0x20;
						}
						code = *(LPDWORD)&huff_tbl[0x100 + (((bit_buffer & 1) << 9) + code) * 4];
						bit_remain--;
						bit_buffer >>= 1;
					}

					if (code == 15)
					{
						break;
					}

					if (code == 0)
					{
						LPBYTE p = huff_tbl;
						while (true)
						{
							if (bit_remain == 0)
							{
								bit_buffer = *(LPDWORD)lpin2; lpin2 += 4;
								bit_remain = 0x20;
							}

							DWORD x = bit_buffer & 1;
							bit_remain--;
							bit_buffer >>= 1;

							if (x == 0)
							{
								break;
							}

							p++;
						}
						i += *p;
					}
					else
					{
						DWORD x = GetNBit(lpin2, code, bit_buffer, bit_remain);
						if (x < (1 << (code - 1)))
							x = x - (1 << code) + 1;
						f[original_order[i]] = x;
						i++;
					}
				}

				f += 64;
			}

			DCT(dct_tbl + 0 * 2, (BYTE*)quant_y);
			DCT(dct_tbl + 64 * 2, (BYTE*)quant_y);
			DCT(dct_tbl + 128 * 2, (BYTE*)quant_y);
			DCT(dct_tbl + 192 * 2, (BYTE*)quant_y);
			DCT(dct_tbl + 256 * 2, (BYTE*)quant_c);
			DCT(dct_tbl + 320 * 2, (BYTE*)quant_c);

			// 2 line by line processing
			// ¡ 
			//   
			LPBYTE dc = lpDst1 - 0x20;
			LPBYTE ac = lpDst1 - 0x20 + ww * 4;
			YCC2RGB(dc, ac, (short*)dct_tbl, (short*)(dct_tbl + 320 * 2), ww * 4);

			//  ¡
			//   
			dc = lpDst1;
			ac = lpDst2 + 0x20 - ww * 4 * 8;
			YCC2RGB(dc, ac, (short*)(dct_tbl + 64 * 2), (short*)(dct_tbl + 324 * 2), ww * 4);

			//   
			// ¡ 
			dc = lpDst1 + ((ww * 4) << 3) - 0x20;
			ac = lpDst2;
			YCC2RGB(dc, ac, (short*)(dct_tbl + 128 * 2), (short*)(dct_tbl + 352 * 2), ww * 4);

			//   
			//  ¡
			dc = lpDst2 + 0x20 - ww * 4;
			ac = lpDst2 + 0x20;
			YCC2RGB(dc, ac, (short*)(dct_tbl + 192 * 2), (short*)(dct_tbl + 356 * 2), ww * 4);

			lpDst1 += 0x40;
			lpDst2 += 0x40;

		} //x_size

		lpout += (ww << 6);
	} //y_size

	LONG lwidth = width * 4;
	LONG lww = ww * 4;
	LPBYTE pDst = &pbtDst[0];
	LPBYTE pDst2 = &dst2[0];

	// Cut and copy extra data
	for (int y = 0; y < height; y++)
	{
		//for (int x = 0; x < lwidth; x++)
		for (int x = 0; x < width; x++)
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
		const BYTE* alpha_end = pbtAlpha + dwAlphaSize;
		while (pbtAlpha < alpha_end)
		{
			if (*pbtAlpha == 0 || *pbtAlpha == 0xFF)
			{
				for (BYTE x = 0; x < pbtAlpha[1]; x++)
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

//////////////////////////////////////////////////////////////////////////////////////////
//

void CJBP1::DCT(BYTE* arg1, BYTE* arg2)
{
	long a, b, c, d;
	long w, x, y, z;
	long s, t, u, v, n;

	BYTE* lp1 = arg1;
	BYTE* lp2 = arg2;
	int   i = 8;

	do
	{
		if (*(short*)&lp1[0x10] == 0 &&
		    *(short*)&lp1[0x20] == 0 &&
		    *(short*)&lp1[0x30] == 0 &&
		    *(short*)&lp1[0x40] == 0 &&
		    *(short*)&lp1[0x50] == 0 &&
		    *(short*)&lp1[0x60] == 0 &&
		    *(short*)&lp1[0x70] == 0)
		{
			*(short*)&lp1[0x00] =
			*(short*)&lp1[0x10] =
			*(short*)&lp1[0x20] =
			*(short*)&lp1[0x30] =
			*(short*)&lp1[0x40] =
			*(short*)&lp1[0x50] =
			*(short*)&lp1[0x60] =
			*(short*)&lp1[0x70] = *(short*)&lp1[0] * *(short*)&lp2[0];
		}
		else
		{
			c = *(short*)&lp2[0x20] * *(short*)&lp1[0x20];
			d = *(short*)&lp2[0x60] * *(short*)&lp1[0x60];
			x = c + d;
			x = (x * 0x00008A8B) >> 16;
			c = ((c * 0x0000C3EF) >> 16) + x; //sar==unsigned‚¾‚¯‚Ç
			d = ((d * (long)0xFFFE26FA) >> 16) + x;
			a = *(short*)&lp1[0x00] * *(short*)&lp2[0x00];
			b = *(short*)&lp1[0x40] * *(short*)&lp2[0x40];
			w = a + b + c; //[esp+20]
			x = a + b - c; //[esp+2C]
			y = a - b + d; //[esp+24]
			z = a - b - d; //[esp+28]

			c = *(short*)&lp1[0x70] * *(short*)&lp2[0x70];
			d = *(short*)&lp1[0x50] * *(short*)&lp2[0x50];
			a = *(short*)&lp1[0x30] * *(short*)&lp2[0x30];
			b = *(short*)&lp1[0x10] * *(short*)&lp2[0x10];
			n = ((a + b + c + d) * 0x00012D06) >> 16;

			u = n + ((c * 0x00004C73) >> 16)
			      + (((c + a) * (long)0xFFFE09D7) >> 16)  //[esp+18]
			      + (((c + b) * (long)0xFFFF199C) >> 16); //[esp+14]

			v = n + ((d * 0x00020D99) >> 16)
			      + (((d + b) * (long)0xFFFF9C1E) >> 16)  //[esp+1C]
			      + (((d + a) * (long)0xFFFD6FE5) >> 16); //[esp+10]

			t = n + ((b * 0x00018056) >> 16)
			      + (((d + b) * (long)0xFFFF9C1E) >> 16)  //[esp+1C]
			      + (((c + b) * (long)0xFFFF199C) >> 16); //[esp+14]

			s = n + ((a * 0x0003129D) >> 16)
			      + (((c + a) * (long)0xFFFE09D7) >> 16)  //[esp+18]
			      + (((d + a) * (long)0xFFFD6FE5) >> 16); //[esp+10]

			*(short*)&lp1[0x00] = (short)(w + t);
			*(short*)&lp1[0x70] = (short)(w - t);
			*(short*)&lp1[0x10] = (short)(y + s);
			*(short*)&lp1[0x60] = (short)(y - s);
			*(short*)&lp1[0x20] = (short)(z + v);
			*(short*)&lp1[0x50] = (short)(z - v);
			*(short*)&lp1[0x30] = (short)(x + u);
			*(short*)&lp1[0x40] = (short)(x - u);
		}

		lp1 += 2;
		lp2 += 2;
		i--;
	} while (i > 0);

	lp1 = arg1;
	i = 8; //[esp+3C] == arg2???

	do
	{
		c = *(short*)&lp1[4];
		d = *(short*)&lp1[12];
		b = *(short*)&lp1[8];
		x = c + d;
		x = ((x * 0x00008A8B) >> 16);
		c = ((c * 0x0000C3EF) >> 16) + x;
		d = ((d * (long)0xFFFE26FA) >> 16) + x;
		a = *(short*)&lp1[0];
		w = a + b + c; //[esp+20]
		x = a + b - c; //[esp+2C]
		y = a - b + d; //[esp+24]
		z = a - b - d; //[esp+28]

		d = *(short*)&lp1[10];
		b = *(short*)&lp1[2];
		c = *(short*)&lp1[14];
		a = *(short*)&lp1[6];
		n = (((a + b + c + d) * 0x00012D06) >> 16);

		s = n + ((a * 0x0003129D) >> 16)
		      + (((a + c) * (long)0xFFFE09D7) >> 16)  //[esp+18]
		      + (((a + d) * (long)0xFFFD6FE5) >> 16); //[esp+10]

		t = n + ((b * 0x00018056) >> 16)
		      + (((b + d) * (long)0xFFFF9C1E) >> 16)
		      + (((b + c) * (long)0xFFFF199C) >> 16); //[esp+14]

		u = n + ((c * 0x00004C73) >> 16)
		      + (((b + c) * (long)0xFFFF199C) >> 16)  //[esp+14]
		      + (((a + c) * (long)0xFFFE09D7) >> 16); //[esp+18]

		v = n + ((d * 0x00020D99) >> 16)
		      + (((b + d) * (long)0xFFFF9C1E) >> 16)
		      + (((a + d) * (long)0xFFFD6FE5) >> 16); //[esp+10]

		*(short*)&lp1[0] = (short)((w + t) >> 3);
		*(short*)&lp1[14] = (short)((w - t) >> 3);
		*(short*)&lp1[2] = (short)((y + s) >> 3);
		*(short*)&lp1[12] = (short)((y - s) >> 3);
		*(short*)&lp1[4] = (short)((z + v) >> 3);
		*(short*)&lp1[10] = (short)((z - v) >> 3);
		*(short*)&lp1[6] = (short)((x + u) >> 3);
		*(short*)&lp1[8] = (short)((x - u) >> 3);

		lp1 += 16;
		i--;
	} while (i > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//

int CJBP1::GetNBit(const BYTE*& lpin, DWORD code, DWORD& bit_buffer, DWORD& bit_remain)
{
	int i = 0;

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
				bit_buffer = *(DWORD*)lpin;
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

//////////////////////////////////////////////////////////////////////////////////////////
//

int CJBP1::MakeTree(BYTE* lp1, int size, DWORD* lp2)
{
	int n, x, c, d;

	BYTE* lp3 = lp1 + 0x900;

	for (int i = 0x102; i > 0; i--)
	{
		*(DWORD*)&lp3[0x800]  = 0;
		*(DWORD*)&lp3[0]      = 0;
		*(DWORD*)&lp3[-0x800] = 0;

		lp3 += 4;
	}

	c = ~size + 1;                //neg [esp+10]
	lp3 = lp1 + size * 4 + 0x900; //[esp+14]

	// Get two minimum frequency values

	for (;;)
	{
		d = n = -100; //-0x64;
		x = 0x7D2B74FF;

		for (int i = 0; i < size; i++)
		{
			if (lp2[i] < x)
			{
				n = i;
				x = lp2[i];
			}
		}

		x = 0x7D2B74FF;

		for (int i = 0; i < size; i++)
		{
			if ((i != n) && (lp2[i] < x))
			{
				d = i;
				x = lp2[i];
			}
		}//440E54

		if ((n < 0) || (d < 0))
		{
			break;
		}

		*(DWORD*)&lp3[-0x800] = n;
		*(DWORD*)&lp3[0] = d;
		*(DWORD*)&lp1[n * 4 + 0x1100] = size;
		*(DWORD*)&lp1[d * 4 + 0x1100] = c;
		lp2[size] = lp2[n] + lp2[d];
		size++;
		c--;
		lp3 += 4;
		lp2[n] = 0x7D2B7500;
		lp2[d] = 0x7D2B7500;
	}

	return	 size;
}

//////////////////////////////////////////////////////////////////////////////////////////
// YCbCr => RGB

void CJBP1::YCC2RGB(BYTE* dc, BYTE* ac, short* Y, short* CbCr, int line)
{
	BYTE*       tbl;
	static bool is_tbl = false;
	static BYTE fixed_byte_tbl[0x300]; //462C48

	// Faster table truncation, saturation
	// -0x80 is less than 0, 0x80 is less than 0xFF 

	if (!is_tbl)
	{
		for (int n = 0; n < 0x100; n++)
			fixed_byte_tbl[n] = 0;

		for (int n = 0; n < 0x100; n++)
			fixed_byte_tbl[n + 0x100] = n;

		for (int n = 0; n<0x100; n++)
			fixed_byte_tbl[n + 0x200] = 0xFF;

		is_tbl = true;
	}

	tbl = fixed_byte_tbl;

	for (int n = 4; n > 0; n--)
	{
		long r, g, b, c, d, w, x, y, z;

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
