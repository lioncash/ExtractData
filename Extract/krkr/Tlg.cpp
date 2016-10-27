#include "stdafx.h"
#include "../../ExtractBase.h"
#include "../../Image.h"
#include "Tlg.h"

#define TVP_TLG6_H_BLOCK_SIZE 8
#define TVP_TLG6_W_BLOCK_SIZE 8

#define TVP_TLG6_GOLOMB_N_COUNT  4
#define TVP_TLG6_LeadingZeroTable_BITS 12
#define TVP_TLG6_LeadingZeroTable_SIZE  (1<<TVP_TLG6_LeadingZeroTable_BITS)
BYTE TVPTLG6LeadingZeroTable[TVP_TLG6_LeadingZeroTable_SIZE];
short TVPTLG6GolombCompressed[TVP_TLG6_GOLOMB_N_COUNT][9] = {
		{3,7,15,27,63,108,223,448,130,},
		{3,5,13,24,51,95,192,384,257,},
		{2,5,12,21,39,86,155,320,384,},
		{2,3,9,18,33,61,129,258,511,},
	/* Tuned by W.Dee, 2004/03/25 */
};
char TVPTLG6GolombBitLengthTable[TVP_TLG6_GOLOMB_N_COUNT*2*128][TVP_TLG6_GOLOMB_N_COUNT];


/// Mount
///
/// @param pclArc Archive
///
bool CTlg::Mount(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".tlg"))
		return false;

	BYTE* pbtHeader = pclArc->GetHed();

	if (memcmp(pbtHeader, "TLG0.0\x00sds\x1a", 11) == 0)
	{
		pbtHeader += 15;
	}

	if (memcmp(pbtHeader, "TLG5.0\x00raw\x1a", 11) == 0)
	{
		// OK
	}
	else if (memcmp(pbtHeader, "TLG6.0\x00raw\x1a", 11) == 0)
	{
		// OK
	}
	else
	{
		// NG

		return false;
	}

	return pclArc->Mount();
}

bool CTlg::Decode(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if (file_info->format != _T("TLG"))
		return false;

	YCMemory<BYTE> buf(file_info->sizeOrg);
	pclArc->Read(&buf[0], file_info->sizeOrg);

	return Decode(pclArc, &buf[0]);
}

bool CTlg::Decode(CArcFile* pclArc, LPBYTE src)
{
	if (memcmp(src, "TLG", 3) != 0)
	{
		pclArc->InitDecrypt( src );
		pclArc->Decrypt(src, pclArc->GetOpenFileInfo()->sizeOrg);
	}

	// Check for TLG0.0 sds
	if (!memcmp(src, "TLG0.0\x00sds\x1a", 11))
		src += 15;

	// Check for TLG raw data
	if (!memcmp(src, "TLG5.0\x00raw\x1a", 11))
		return DecompTLG5(pclArc, &src[11]);
	if (!memcmp(src, "TLG6.0\x00raw\x1a", 11))
		return DecompTLG6(pclArc, &src[11]);

	pclArc->OpenFile();
	pclArc->WriteFile(src, pclArc->GetOpenFileInfo()->sizeCmp);
	return true;
}

bool CTlg::DecompTLG5(CArcFile* pclArc, LPBYTE src)
{
	BYTE colors = src[0];
	LONG width = *(LPLONG)&src[1];
	LONG height = *(LPLONG)&src[5];
	LONG blockHeight = *(LPLONG)&src[9];

	if (colors != 3 && colors != 4)
		return false;

	DWORD blockCount = ((height - 1) / blockHeight) + 1;
	src += 13 + blockCount * 4;
	DWORD r = 0;

	// Ensure output buffer
	DWORD dstSize = width * height * colors;
	YCMemory<BYTE> dst(dstSize);
	LPBYTE pdst = &dst[0];
	memset(pdst, 0, dstSize);

	// Ensure buffer dictionary
	YCMemory<BYTE> dic(4096);
	memset(&dic[0], 0, 4096);

	// Ensure each buffer to store RGBA data
	YCMemory<BYTE> outbufs[4];
	for (BYTE i = 0; i < colors; i++)
	{
		outbufs[i].resize(blockHeight * width + 10);
	}

	LPBYTE prevline = &dst[0]; // Get a buffer filled with 0's
	for (LONG y_blk = 0; y_blk < height; y_blk += blockHeight)
	{
		// Decompress
		for (BYTE c = 0; c < colors; c++)
		{
			BYTE comp = src[0];
			DWORD size = *(LPDWORD)&src[1];
			src += 5;

			// Modified LZSS compressed data
			if (comp == 0)
				r = DecompLZSS(&outbufs[c][0], src, size, &dic[0], r);
			// Raw data
			else
				memcpy(&outbufs[c][0], src, size);
			src += size;
		}

		// Compose colors and store
		LONG y_lim = y_blk + blockHeight;
		if (y_lim > height) y_lim = height;
		LPBYTE poutbufs[4];

		for (DWORD c = 0; c < colors; c++)
		{
			poutbufs[c] = &outbufs[c][0];
		}

		for (LONG y = y_blk; y < y_lim; y++)
		{
			ComposeColors(pdst, prevline, poutbufs, width, colors);
			prevline = pdst;
			pdst += width * colors;

			for (DWORD i = 0; i < colors; i++)
			{
				poutbufs[i] += width;
			}
		}
	}

	// BMP Output
	CImage image;
	image.Init(pclArc, width, height, colors << 3);
	image.WriteReverse(&dst[0], dstSize);

	return true;
}

bool CTlg::DecompTLG6(CArcFile* pclArc, LPBYTE src)
{
	BYTE colors = src[0];

	if (colors != 1 && colors != 4 && colors != 3)
		return false;
	if (src[1] != 0) // Data flag
		return false;
	if (src[2] != 0) // Color type (currently always zero)
		return false;
	if (src[3] != 0) // External golomb table (currently always zero)
		return false;

	LONG width = *(LPLONG)&src[4];
	LONG height = *(LPLONG)&src[8];
	DWORD max_bit_length = *(LPDWORD)&src[12];
	src += 16;

	// Compute some values
	InitTLG6Table();
	DWORD x_block_count = (width - 1) / TVP_TLG6_W_BLOCK_SIZE + 1;
	DWORD y_block_count = (height - 1) / TVP_TLG6_H_BLOCK_SIZE + 1;
	DWORD main_count = width / TVP_TLG6_W_BLOCK_SIZE;
	DWORD fraction = width - main_count * TVP_TLG6_W_BLOCK_SIZE;

	// Ensure output buffer
	DWORD dstSize = width * height * 4;
	YCMemory<BYTE> dst(dstSize);
	LPBYTE pdst = &dst[0];

	// Allocate memory
	YCMemory<BYTE> bit_pool(max_bit_length / 8 + 5);
	YCMemory<DWORD> pixelbuf(sizeof(DWORD) * width * TVP_TLG6_H_BLOCK_SIZE + 1);
	YCMemory<BYTE> filter_types(x_block_count * y_block_count);
	YCMemory<DWORD> zeroline(width, (colors == 3) ? 0xff000000 : 0x00000000);
	YCMemory<BYTE> LZSS_dic(4096);

	// Initialize LZSS text (used by chroma filter type codes)
	{
		LPDWORD p = (LPDWORD)&LZSS_dic[0];
		DWORD i_max = 0x01010101 << 5;
		DWORD j_max = 0x01010101 << 4;

		for (DWORD i = 0; i < i_max; i += 0x01010101)
		{
			for (DWORD j = 0; j < j_max; j += 0x01010101)
			{
				p[0] = i, p[1] = j, p += 2;
			}
		}
	}

	// Read chroma filter types.
	// Chroma filter types are compressed via LZSS as used by TLG5.
	{
		DWORD src_size = *(LPDWORD)&src[0];
		src += 4;
		DecompLZSS(&filter_types[0], src, src_size, &LZSS_dic[0], 0);
		src += src_size;
	}

	// For each horizontal block group ...
	LPDWORD prevline = &zeroline[0];
	for (LONG y = 0; y < height; y += TVP_TLG6_H_BLOCK_SIZE)
	{
		LONG ylim = y + TVP_TLG6_H_BLOCK_SIZE;
		if (ylim > height) ylim = height;

		DWORD pixel_count = (ylim - y) * width;

		// Decode values
		for (BYTE c = 0; c < colors; c++)
		{
			// Read bit length
			DWORD bit_length = *(LPDWORD)&src[0];
			src += 4;

			// Get compress method
			DWORD method = (bit_length >> 30) & 3;
			bit_length &= 0x3fffffff;

			// Compute byte length
			DWORD byte_length = bit_length / 8;
			if (bit_length % 8) byte_length++;

			// Read source from input
			memcpy(&bit_pool[0], src, byte_length);
			src += byte_length;

			// Decode values
			// Two most significant bits of bitlength are
			// entropy coding method;
			// 00 means Golomb method,
			// 01 means Gamma method (not yet supported),
			// 10 means modified LZSS method (not yet supported),
			// 11 means raw (uncompressed) data (not yet supported).

			switch (method)
			{
			case 0:
				TVPTLG6DecodeGolombValues((LPBYTE)&pixelbuf[0] + c, pixel_count, &bit_pool[0], c);
				break;
			default:
				return false;
			}
		}

		// For each line
		LPBYTE ft = &filter_types[0] + (y / TVP_TLG6_H_BLOCK_SIZE) * x_block_count;
		int skipbytes = (ylim - y) * TVP_TLG6_W_BLOCK_SIZE;

		for (LONG yy = y; yy < ylim; yy++)
		{
			LPDWORD curline = (LPDWORD)pdst;

			int dir = (yy & 1) ^ 1;
			int oddskip = (ylim - yy -1) - (yy - y);

			if (main_count)
			{
				int start = ((width < TVP_TLG6_W_BLOCK_SIZE) ? width : TVP_TLG6_W_BLOCK_SIZE) * (yy - y);
				TVPTLG6DecodeLine(prevline, curline, width, main_count, ft, skipbytes, &pixelbuf[0] + start, colors == 3 ? 0xff000000 : 0, oddskip, dir);
			}

			if (main_count != x_block_count)
			{
				int ww = fraction;
				if (ww > TVP_TLG6_W_BLOCK_SIZE) ww = TVP_TLG6_W_BLOCK_SIZE;
				int start = ww * (yy - y);
				TVPTLG6DecodeLineGeneric(prevline, curline, width, main_count, x_block_count, ft, skipbytes, &pixelbuf[0] + start, colors == 3 ? 0xff000000 : 0, oddskip, dir);
			}

			prevline = curline;
			pdst += width * 4;
		}
	}

	// BMP Output
	WORD bpp = colors << 3;
	if (bpp == 24)
		bpp = 32;

	CImage image;
	image.Init(pclArc, width, height, bpp);
	image.WriteReverse(&dst[0], dstSize);

	return true;
}

// LZSS Decompression
DWORD CTlg::DecompLZSS(LPBYTE out, LPBYTE in, DWORD insize, LPBYTE dic, DWORD initialr)
{
	DWORD r = initialr;
	DWORD flags = 0;
	const LPBYTE inlim = in + insize;
	
	while (in < inlim)
	{
		if (((flags >>= 1) & 256) == 0)
			flags = *in++ | 0xff00;

		if (flags & 1)
		{
			int mpos = in[0] | ((in[1] & 0xf) << 8);
			int mlen = (in[1] & 0xf0) >> 4;
			in += 2;
			mlen += 3;
			if (mlen == 18) mlen += *in++;

			while (mlen--)
			{
				*out++ = dic[r++] = dic[mpos++];
				mpos &= (4096 - 1);
				r &= (4096 - 1);
			}
		}
		else
		{
			BYTE c = *in++;
			*out++ = c;
			dic[r++] = c;
			r &= (4096 - 1);
		}
	}

	return r;
}

// RGBA Composition
void CTlg::ComposeColors(LPBYTE outp, LPBYTE upper, LPBYTE const* buf, DWORD width, DWORD colors)
{
	BYTE c[4]  = {0, 0, 0, 0};
	BYTE pc[4] = {0, 0, 0, 0};

	for (DWORD x = 0; x < width; x++)
	{
		for (DWORD i = 0; i < colors; i++)
			c[i] = buf[i][x];

		c[0] += c[1];
		c[2] += c[1];

		for (DWORD i = 0; i < colors; i++)
			outp[i] = (pc[i] += c[i]) + upper[i];

		outp += colors;
		upper += colors;
	}
}

void CTlg::TVPTLG6DecodeGolombValues(LPBYTE pixelbuf, DWORD pixel_count, LPBYTE bit_pool, BYTE color)
{
	/*
		Decode values packed in "bit_pool".
		values are coded using golomb code.
	*/

	int n = TVP_TLG6_GOLOMB_N_COUNT - 1; /* Output counter */
	int a = 0; /* Summary of absolute values of errors */

	int bit_pos = 1;
	BYTE zero = (*bit_pool & 1) ? 0 : 1;

	BYTE *limit = pixelbuf + pixel_count * 4;

	while (pixelbuf < limit)
	{
		// Get running count
		int count;

		{
			DWORD t = *(LPDWORD)bit_pool >> bit_pos;
			int b = TVPTLG6LeadingZeroTable[t & (TVP_TLG6_LeadingZeroTable_SIZE - 1)];
			int bit_count = b;
			while (!b)
			{
				bit_count += TVP_TLG6_LeadingZeroTable_BITS;
				bit_pos += TVP_TLG6_LeadingZeroTable_BITS;
				bit_pool += bit_pos >> 3;
				bit_pos &= 7;
				t = *(LPDWORD)bit_pool >> bit_pos;
				b = TVPTLG6LeadingZeroTable[t & (TVP_TLG6_LeadingZeroTable_SIZE - 1)];
				bit_count += b;
			}

			bit_pos += b;
			bit_pool += bit_pos >> 3;
			bit_pos &= 7;

			bit_count--;
			count = 1 << bit_count;
			count += (*(LPDWORD)bit_pool >> bit_pos) & (count - 1);

			bit_pos += bit_count;
			bit_pool += bit_pos >> 3;
			bit_pos &= 7;
		}

		if (zero)
		{
			// Zero values

			// Fill destination with zero
			do
			{
				if (color == 0)
					*(LPDWORD)pixelbuf = 0;
				else
					*pixelbuf = 0;

				pixelbuf += 4;

			} while (--count);

			zero ^= 1;
		}
		else
		{
			// Non-zero values

			// Fill destination with golomb code

			do
			{
				DWORD t = *(LPDWORD)bit_pool >> bit_pos;
				int bit_count;
				int b;
				if (t)
				{
					b = TVPTLG6LeadingZeroTable[t & (TVP_TLG6_LeadingZeroTable_SIZE - 1)];
					bit_count = b;
					while (!b)
					{
						bit_count += TVP_TLG6_LeadingZeroTable_BITS;
						bit_pos += TVP_TLG6_LeadingZeroTable_BITS;
						bit_pool += bit_pos >> 3;
						bit_pos &= 7;
						t = *(DWORD *)bit_pool >> bit_pos;
						b = TVPTLG6LeadingZeroTable[t & (TVP_TLG6_LeadingZeroTable_SIZE - 1)];
						bit_count += b;
					}

					bit_count--;
				}
				else
				{
					bit_pool += 5;
					bit_count = bit_pool[-1];
					bit_pos = 0;
					t = *(DWORD *)bit_pool;
					b = 0;
				}

				int k = TVPTLG6GolombBitLengthTable[a][n];
				int v = (bit_count << k) + ((t >> b) & ((1 << k) - 1));
				int sign = (v & 1) - 1;
				v >>= 1;
				a += v;

				if (color == 0)
					*(LPDWORD)pixelbuf = (BYTE)((v ^ sign) + sign + 1);
				else
					*pixelbuf = (BYTE)((v ^ sign) + sign + 1);

				pixelbuf += 4;

				bit_pos += b;
				bit_pos += k;
				bit_pool += bit_pos >> 3;
				bit_pos &= 7;

				if (--n < 0)
				{
					a >>= 1;  n = TVP_TLG6_GOLOMB_N_COUNT - 1;
				}

			} while (--count);
			zero ^= 1;
		}
	}
}

#define TVP_TLG6_DO_CHROMA_DECODE_PROTO(B, G, R, A, POST_INCREMENT) do \
			{ \
				DWORD u = *prevline; \
				p = med(p, u, up, \
					(0xff0000 & ((R)<<16)) + (0xff00 & ((G)<<8)) + (0xff & (B)) + ((A) << 24) ); \
				up = u; \
				*curline = p; \
				curline++; \
				prevline++; \
				POST_INCREMENT \
			} while(--w);
#define TVP_TLG6_DO_CHROMA_DECODE_PROTO2(B, G, R, A, POST_INCREMENT) do \
			{ \
				DWORD u = *prevline; \
				p = avg(p, u, up, \
					(0xff0000 & ((R)<<16)) + (0xff00 & ((G)<<8)) + (0xff & (B)) + ((A) << 24) ); \
				up = u; \
				*curline = p; \
				curline++; \
				prevline++; \
				POST_INCREMENT \
			} while(--w);
#define TVP_TLG6_DO_CHROMA_DECODE(N, R, G, B) case (N<<1): \
	TVP_TLG6_DO_CHROMA_DECODE_PROTO(R, G, B, IA, {in+=step;}) break; \
	case (N<<1)+1: \
	TVP_TLG6_DO_CHROMA_DECODE_PROTO2(R, G, B, IA, {in+=step;}) break;

void CTlg::TVPTLG6DecodeLineGeneric(LPDWORD prevline, DWORD *curline, DWORD width, DWORD start_block, DWORD block_limit, LPBYTE filtertypes, DWORD skipblockbytes, LPDWORD in, DWORD initialp, DWORD oddskip, DWORD dir)
{
	/*
		Chroma/luminosity decoding
		(this does reordering, color correlation filter, MED/AVG  at a time)
	*/
	DWORD p, up;

	if (start_block)
	{
		prevline += start_block * TVP_TLG6_W_BLOCK_SIZE;
		curline  += start_block * TVP_TLG6_W_BLOCK_SIZE;
		p  = curline[-1];
		up = prevline[-1];
	}
	else
	{
		p = up = initialp;
	}

	in += skipblockbytes * start_block;
	int step = (dir & 1) ? 1 : -1;

	for (DWORD i = start_block; i < block_limit; i++)
	{
		int w = width - i * TVP_TLG6_W_BLOCK_SIZE;

		if (w > TVP_TLG6_W_BLOCK_SIZE) 
			w = TVP_TLG6_W_BLOCK_SIZE;

		int ww = w;

		if (step == -1) 
			in += ww - 1;

		if (i & 1) 
			in += oddskip * ww;

		switch (filtertypes[i])
		{
#define IA  (char)((*in >> 24) & 0xff)
#define IR  (char)((*in >> 16) & 0xff)
#define IG  (char)((*in >>  8) & 0xff)
#define IB  (char)((*in      ) & 0xff)
		TVP_TLG6_DO_CHROMA_DECODE( 0, IB, IG, IR); 
		TVP_TLG6_DO_CHROMA_DECODE( 1, IB+IG, IG, IR+IG); 
		TVP_TLG6_DO_CHROMA_DECODE( 2, IB, IG+IB, IR+IB+IG); 
		TVP_TLG6_DO_CHROMA_DECODE( 3, IB+IR+IG, IG+IR, IR); 
		TVP_TLG6_DO_CHROMA_DECODE( 4, IB+IR, IG+IB+IR, IR+IB+IR+IG); 
		TVP_TLG6_DO_CHROMA_DECODE( 5, IB+IR, IG+IB+IR, IR); 
		TVP_TLG6_DO_CHROMA_DECODE( 6, IB+IG, IG, IR); 
		TVP_TLG6_DO_CHROMA_DECODE( 7, IB, IG+IB, IR); 
		TVP_TLG6_DO_CHROMA_DECODE( 8, IB, IG, IR+IG); 
		TVP_TLG6_DO_CHROMA_DECODE( 9, IB+IG+IR+IB, IG+IR+IB, IR+IB); 
		TVP_TLG6_DO_CHROMA_DECODE(10, IB+IR, IG+IR, IR); 
		TVP_TLG6_DO_CHROMA_DECODE(11, IB, IG+IB, IR+IB); 
		TVP_TLG6_DO_CHROMA_DECODE(12, IB, IG+IR+IB, IR+IB); 
		TVP_TLG6_DO_CHROMA_DECODE(13, IB+IG, IG+IR+IB+IG, IR+IB+IG); 
		TVP_TLG6_DO_CHROMA_DECODE(14, IB+IG+IR, IG+IR, IR+IB+IG+IR); 
		TVP_TLG6_DO_CHROMA_DECODE(15, IB, IG+(IB<<1), IR+(IB<<1));

		default:
			return;
		}

		if (step == 1)
			in += skipblockbytes - ww;
		else
			in += skipblockbytes + 1;

		if (i & 1)
			in -= oddskip * ww;
#undef IR
#undef IG
#undef IB
	}
}

void CTlg::TVPTLG6DecodeLine(LPDWORD prevline, LPDWORD curline, DWORD width, DWORD block_count, LPBYTE filtertypes, DWORD skipblockbytes, LPDWORD in, DWORD initialp, DWORD oddskip, DWORD dir)
{
	TVPTLG6DecodeLineGeneric(prevline, curline, width, 0, block_count, filtertypes, skipblockbytes, in, initialp, oddskip, dir);
}

DWORD CTlg::make_gt_mask(DWORD a, DWORD b)
{
	DWORD tmp2 = ~b;
	DWORD tmp = ((a & tmp2) + (((a ^ tmp2) >> 1) & 0x7f7f7f7f) ) & 0x80808080;
	tmp = ((tmp >> 7) + 0x7f7f7f7f) ^ 0x7f7f7f7f;
	return tmp;
}

DWORD CTlg::packed_bytes_add(DWORD a, DWORD b)
{
	DWORD tmp = (((a & b)<<1) + ((a ^ b) & 0xfefefefe) ) & 0x01010100;
	return a + b - tmp;
}

DWORD CTlg::med2(DWORD a, DWORD b, DWORD c)
{
	/* Do Median Edge Detector   thx, Mr. sugi  at    kirikiri.info */
	DWORD aa_gt_bb = make_gt_mask(a, b);
	DWORD a_xor_b_and_aa_gt_bb = ((a ^ b) & aa_gt_bb);
	DWORD aa = a_xor_b_and_aa_gt_bb ^ a;
	DWORD bb = a_xor_b_and_aa_gt_bb ^ b;
	DWORD n = make_gt_mask(c, bb);
	DWORD nn = make_gt_mask(aa, c);
	DWORD m = ~(n | nn);
	return (n & aa) | (nn & bb) | ((bb & m) - (c & m) + (aa & m));
}

DWORD CTlg::med(DWORD a, DWORD b, DWORD c, DWORD v)
{
	return packed_bytes_add(med2(a, b, c), v);
}

#define TLG6_AVG_PACKED(x, y) ((((x) & (y)) + ((((x) ^ (y)) & 0xfefefefe) >> 1)) +\
			(((x)^(y))&0x01010101))

DWORD CTlg::avg(DWORD a, DWORD b, DWORD c, DWORD v)
{
	return packed_bytes_add(TLG6_AVG_PACKED(a, b), v);
}

void CTlg::InitTLG6Table()
{
	/* Table which indicates first set bit position + 1. */
	/* This may be replaced by BSF (IA32 instrcution). */

	for (int i = 0; i < TVP_TLG6_LeadingZeroTable_SIZE; i++)
	{
		int cnt = 0;
		int j;

		for (j = 1; j != TVP_TLG6_LeadingZeroTable_SIZE && !(i & j);
			j <<= 1, cnt++);

		cnt++;

		if (j == TVP_TLG6_LeadingZeroTable_SIZE)
			cnt = 0;

		TVPTLG6LeadingZeroTable[i] = cnt;
	}

	memset(TVPTLG6GolombBitLengthTable, 0, TVP_TLG6_GOLOMB_N_COUNT*2*128 * TVP_TLG6_GOLOMB_N_COUNT);
	for (int n = 0; n < TVP_TLG6_GOLOMB_N_COUNT; n++)
	{
		int a = 0;
		for (int i = 0; i < 9; i++)
		{
			for (int j = 0; j < TVPTLG6GolombCompressed[n][i]; j++)
			{
				TVPTLG6GolombBitLengthTable[a++][n] = (char)i;
			}
		}
	}
}
