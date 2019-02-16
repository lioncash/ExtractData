#include "StdAfx.h"
#include "Extract/krkr/Tlg.h"

#include "ArcFile.h"
#include "Image.h"

namespace
{
#define TVP_TLG6_H_BLOCK_SIZE 8
#define TVP_TLG6_W_BLOCK_SIZE 8

#define TVP_TLG6_GOLOMB_N_COUNT  4
#define TVP_TLG6_LeadingZeroTable_BITS 12
#define TVP_TLG6_LeadingZeroTable_SIZE  (1<<TVP_TLG6_LeadingZeroTable_BITS)
u8 TVPTLG6LeadingZeroTable[TVP_TLG6_LeadingZeroTable_SIZE];
constexpr s16 TVPTLG6GolombCompressed[TVP_TLG6_GOLOMB_N_COUNT][9] = {
	{3,7,15,27,63,108,223,448,130},
	{3,5,13,24,51,95,192,384,257},
	{2,5,12,21,39,86,155,320,384},
	{2,3,9,18,33,61,129,258,511}
	// Tuned by W.Dee, 2004/03/25
};
char TVPTLG6GolombBitLengthTable[TVP_TLG6_GOLOMB_N_COUNT*2*128][TVP_TLG6_GOLOMB_N_COUNT];
} // Anonymous namespace

/// Mount
///
/// @param archive Archive
///
bool CTlg::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".tlg"))
		return false;

	const u8* header = archive->GetHeader();

	if (memcmp(header, "TLG0.0\x00sds\x1a", 11) == 0)
	{
		header += 15;
	}

	if (memcmp(header, "TLG5.0\x00raw\x1a", 11) == 0)
	{
		// OK
	}
	else if (memcmp(header, "TLG6.0\x00raw\x1a", 11) == 0)
	{
		// OK
	}
	else
	{
		// NG
		return false;
	}

	return archive->Mount();
}

bool CTlg::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("TLG"))
		return false;

	std::vector<u8> buffer(file_info->size_org);
	archive->Read(buffer.data(), buffer.size());

	return Decode(archive, buffer.data());
}

bool CTlg::Decode(CArcFile* archive, u8* src)
{
	if (memcmp(src, "TLG", 3) != 0)
	{
		archive->InitDecrypt(src);
		archive->Decrypt(src, archive->GetOpenFileInfo()->size_org);
	}

	// Check for TLG0.0 sds
	if (!memcmp(src, "TLG0.0\x00sds\x1a", 11))
		src += 15;

	// Check for TLG raw data
	if (!memcmp(src, "TLG5.0\x00raw\x1a", 11))
		return DecompTLG5(archive, &src[11]);
	if (!memcmp(src, "TLG6.0\x00raw\x1a", 11))
		return DecompTLG6(archive, &src[11]);

	archive->OpenFile();
	archive->WriteFile(src, archive->GetOpenFileInfo()->size_cmp);
	return true;
}

bool CTlg::DecompTLG5(CArcFile* archive, const u8* src)
{
	const u8 colors = src[0];
	const s32 width = *reinterpret_cast<const s32*>(&src[1]);
	const s32 height = *reinterpret_cast<const s32*>(&src[5]);
	const s32 block_height = *reinterpret_cast<const s32*>(&src[9]);

	if (colors != 3 && colors != 4)
		return false;

	const u32 block_count = ((height - 1) / block_height) + 1;
	src += 13 + block_count * 4;
	u32 r = 0;

	// Ensure output buffer
	const u32 dst_size = static_cast<u32>(width * height * colors);
	std::vector<u8> dst(dst_size);
	u8* dst_ptr = dst.data();

	// Ensure dictionary buffer
	std::vector<u8> dictionary(4096);

	// Ensure each buffer to store RGBA data is created
	std::array<std::vector<u8>, 4> out_buffers;
	for (size_t i = 0; i < colors; i++)
	{
		out_buffers[i].resize(block_height * width + 10);
	}

	const u8* prev_line = dst.data(); // Get a buffer filled with 0's
	for (s32 y_blk = 0; y_blk < height; y_blk += block_height)
	{
		// Decompress
		for (size_t c = 0; c < colors; c++)
		{
			const u8 comp = src[0];
			const u32 size = *reinterpret_cast<const u32*>(&src[1]);
			src += 5;

			// Modified LZSS compressed data
			if (comp == 0)
				r = DecompLZSS(out_buffers[c].data(), src, size, dictionary.data(), r);
			else // Raw data
				memcpy(out_buffers[c].data(), src, size);

			src += size;
		}

		// Compose colors and store
		s32 y_lim = y_blk + block_height;
		if (y_lim > height)
			y_lim = height;

		std::array<const u8*, 4> out_buffer_ptrs;
		for (size_t c = 0; c < colors; c++)
		{
			out_buffer_ptrs[c] = out_buffers[c].data();
		}

		for (s32 y = y_blk; y < y_lim; y++)
		{
			ComposeColors(dst_ptr, prev_line, out_buffer_ptrs.data(), width, colors);
			prev_line = dst_ptr;
			dst_ptr += width * colors;

			for (size_t i = 0; i < colors; i++)
			{
				out_buffer_ptrs[i] += width;
			}
		}
	}

	// BMP Output
	CImage image;
	image.Init(archive, width, height, colors << 3);
	image.WriteReverse(dst.data(), dst.size());

	return true;
}

bool CTlg::DecompTLG6(CArcFile* archive, const u8* src)
{
	const u8 colors = src[0];

	if (colors != 1 && colors != 4 && colors != 3)
		return false;
	if (src[1] != 0) // Data flag
		return false;
	if (src[2] != 0) // Color type (currently always zero)
		return false;
	if (src[3] != 0) // External golomb table (currently always zero)
		return false;

	const s32 width = *reinterpret_cast<const s32*>(&src[4]);
	const s32 height = *reinterpret_cast<const s32*>(&src[8]);
	const u32 max_bit_length = *reinterpret_cast<const u32*>(&src[12]);
	src += 16;

	// Compute some values
	InitTLG6Table();
	const u32 x_block_count = (width - 1) / TVP_TLG6_W_BLOCK_SIZE + 1;
	const u32 y_block_count = (height - 1) / TVP_TLG6_H_BLOCK_SIZE + 1;
	const u32 main_count = width / TVP_TLG6_W_BLOCK_SIZE;
	const u32 fraction = width - main_count * TVP_TLG6_W_BLOCK_SIZE;

	// Ensure output buffer
	const u32 dst_size = width * height * 4;
	std::vector<u8> dst(dst_size);
	u8* pdst = dst.data();

	// Allocate memory
	std::vector<u8> bit_pool(max_bit_length / 8 + 5);
	std::vector<u32> pixel_buffer(sizeof(u32) * width * TVP_TLG6_H_BLOCK_SIZE + 1);
	std::vector<u8> filter_types(x_block_count * y_block_count);
	std::vector<u32> zero_line(width, (colors == 3) ? 0xff000000 : 0x00000000);
	std::vector<u8> LZSS_dic(4096);

	// Initialize LZSS text (used by chroma filter type codes)
	{
		u32* p = reinterpret_cast<u32*>(LZSS_dic.data());
		const u32 i_max = 0x01010101 << 5;
		const u32 j_max = 0x01010101 << 4;

		for (u32 i = 0; i < i_max; i += 0x01010101)
		{
			for (u32 j = 0; j < j_max; j += 0x01010101)
			{
				p[0] = i;
				p[1] = j;
				p += 2;
			}
		}
	}

	// Read chroma filter types.
	// Chroma filter types are compressed via LZSS as used by TLG5.
	{
		const u32 src_size = *reinterpret_cast<const u32*>(&src[0]);
		src += 4;
		DecompLZSS(filter_types.data(), src, src_size, LZSS_dic.data(), 0);
		src += src_size;
	}

	// For each horizontal block group ...
	const u32* prev_line = zero_line.data();
	for (s32 y = 0; y < height; y += TVP_TLG6_H_BLOCK_SIZE)
	{
		s32 ylim = y + TVP_TLG6_H_BLOCK_SIZE;
		if (ylim > height)
			ylim = height;

		const u32 pixel_count = (ylim - y) * width;

		// Decode values
		for (u8 c = 0; c < colors; c++)
		{
			// Read bit length
			u32 bit_length = *reinterpret_cast<const u32*>(&src[0]);
			src += 4;

			// Get compression method
			const u32 method = (bit_length >> 30) & 3;
			bit_length &= 0x3fffffff;

			// Compute byte length
			u32 byte_length = bit_length / 8;
			if (bit_length % 8)
				byte_length++;

			// Read source from input
			memcpy(bit_pool.data(), src, byte_length);
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
				TVPTLG6DecodeGolombValues(reinterpret_cast<u8*>(pixel_buffer.data()) + c, pixel_count, bit_pool.data(), c);
				break;
			default:
				return false;
			}
		}

		// For each line
		const u8* ft = &filter_types[0] + (y / TVP_TLG6_H_BLOCK_SIZE) * x_block_count;
		const s32 skip_bytes = (ylim - y) * TVP_TLG6_W_BLOCK_SIZE;

		for (s32 yy = y; yy < ylim; yy++)
		{
			u32* current_line = reinterpret_cast<u32*>(pdst);

			const s32 dir = (yy & 1) ^ 1;
			const s32 odd_skip = (ylim - yy -1) - (yy - y);

			if (main_count != 0)
			{
				const s32 start = ((width < TVP_TLG6_W_BLOCK_SIZE) ? width : TVP_TLG6_W_BLOCK_SIZE) * (yy - y);
				TVPTLG6DecodeLine(prev_line, current_line, width, main_count,
				                  ft, skip_bytes,
				                  pixel_buffer.data() + start,
				                  colors == 3 ? 0xff000000 : 0,
				                  odd_skip, dir);
			}

			if (main_count != x_block_count)
			{
				s32 ww = fraction;
				if (ww > TVP_TLG6_W_BLOCK_SIZE)
					ww = TVP_TLG6_W_BLOCK_SIZE;

				const s32 start = ww * (yy - y);
				TVPTLG6DecodeLineGeneric(prev_line, current_line, width,
				                         main_count, x_block_count, ft,
				                         skip_bytes,
				                         pixel_buffer.data() + start,
				                         colors == 3 ? 0xff000000 : 0,
				                         odd_skip, dir);
			}

			prev_line = current_line;
			pdst += width * 4;
		}
	}

	// BMP Output
	u16 bpp = colors << 3;
	if (bpp == 24)
		bpp = 32;

	CImage image;
	image.Init(archive, width, height, bpp);
	image.WriteReverse(dst.data(), dst.size());

	return true;
}

// LZSS Decompression
u32 CTlg::DecompLZSS(u8* dst, const u8* src, size_t src_size, u8* dictionary, u32 initial_r)
{
	u32 r = initial_r;
	u32 flags = 0;
	const u8* src_end = src + src_size;
	
	while (src < src_end)
	{
		if (((flags >>= 1) & 256) == 0)
			flags = *src++ | 0xff00;

		if (flags & 1)
		{
			s32 mpos = src[0] | ((src[1] & 0xf) << 8);
			s32 mlen = (src[1] & 0xf0) >> 4;
			src += 2;
			mlen += 3;
			if (mlen == 18)
				mlen += *src++;

			while (mlen--)
			{
				*dst++ = dictionary[r++] = dictionary[mpos++];
				mpos &= (4096 - 1);
				r &= (4096 - 1);
			}
		}
		else
		{
			const u8 c = *src++;
			*dst++ = c;
			dictionary[r++] = c;
			r &= 4096 - 1;
		}
	}

	return r;
}

// RGBA Composition
void CTlg::ComposeColors(u8* dst, const u8* upper, const u8* const* buffer, u32 width, u32 colors)
{
	std::array<u8, 4> c{};
	std::array<u8, 4> pc{};

	for (size_t x = 0; x < width; x++)
	{
		for (size_t i = 0; i < colors; i++)
			c[i] = buffer[i][x];

		c[0] += c[1];
		c[2] += c[1];

		for (size_t i = 0; i < colors; i++)
		{
			pc[i] += c[i];
			dst[i] = pc[i] + upper[i];
		}

		dst += colors;
		upper += colors;
	}
}

void CTlg::TVPTLG6DecodeGolombValues(u8* pixelbuf, u32 pixel_count, const u8* bit_pool, u8 color)
{
	// Decode values packed in "bit_pool".
	// values are coded using golomb code.

	s32 n = TVP_TLG6_GOLOMB_N_COUNT - 1; // Output counter
	s32 a = 0; // Summary of absolute values of errors

	s32 bit_pos = 1;
	u8 zero = (*bit_pool & 1) ? 0 : 1;

	const u8 *limit = pixelbuf + pixel_count * 4;

	while (pixelbuf < limit)
	{
		// Get running count
		s32 count;

		{
			u32 t = *reinterpret_cast<const u32*>(bit_pool) >> bit_pos;
			s32 b = TVPTLG6LeadingZeroTable[t & (TVP_TLG6_LeadingZeroTable_SIZE - 1)];
			s32 bit_count = b;
			while (!b)
			{
				bit_count += TVP_TLG6_LeadingZeroTable_BITS;
				bit_pos += TVP_TLG6_LeadingZeroTable_BITS;
				bit_pool += bit_pos >> 3;
				bit_pos &= 7;
				t = *reinterpret_cast<const u32*>(bit_pool) >> bit_pos;
				b = TVPTLG6LeadingZeroTable[t & (TVP_TLG6_LeadingZeroTable_SIZE - 1)];
				bit_count += b;
			}

			bit_pos += b;
			bit_pool += bit_pos >> 3;
			bit_pos &= 7;

			bit_count--;
			count = 1 << bit_count;
			count += (*reinterpret_cast<const u32*>(bit_pool) >> bit_pos) & (count - 1);

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
					std::memset(pixelbuf, 0, sizeof(u32));
				else
					*pixelbuf = 0;

				pixelbuf += sizeof(u32);

			} while (--count);

			zero ^= 1;
		}
		else
		{
			// Non-zero values
			// Fill destination with golomb code
			do
			{
				u32 t = *reinterpret_cast<const u32*>(bit_pool) >> bit_pos;
				s32 bit_count;
				s32 b;
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
						t = *reinterpret_cast<const u32*>(bit_pool) >> bit_pos;
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
					t = *reinterpret_cast<const u32*>(bit_pool);
					b = 0;
				}

				s32 k = TVPTLG6GolombBitLengthTable[a][n];
				s32 v = (bit_count << k) + ((t >> b) & ((1 << k) - 1));
				s32 sign = (v & 1) - 1;
				v >>= 1;
				a += v;

				if (color == 0)
					*reinterpret_cast<u32*>(pixelbuf) = static_cast<u8>((v ^ sign) + sign + 1);
				else
					*pixelbuf = static_cast<u8>((v ^ sign) + sign + 1);

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
				const u32 u = *prev_line; \
				p = med(p, u, up, \
					(0xff0000 & ((R) << 16)) + (0xff00 & ((G) << 8)) + (0xff & (B)) + ((A) << 24) ); \
				up = u; \
				*current_line = p; \
				current_line++; \
				prev_line++; \
				POST_INCREMENT \
			} while (--w);
#define TVP_TLG6_DO_CHROMA_DECODE_PROTO2(B, G, R, A, POST_INCREMENT) do \
			{ \
				const u32 u = *prev_line; \
				p = avg(p, u, up, \
					(0xff0000 & ((R) << 16)) + (0xff00 & ((G) << 8)) + (0xff & (B)) + ((A) << 24) ); \
				up = u; \
				*current_line = p; \
				current_line++; \
				prev_line++; \
				POST_INCREMENT \
			} while (--w);
#define TVP_TLG6_DO_CHROMA_DECODE(N, R, G, B) \
	case ((N) << 1): \
		TVP_TLG6_DO_CHROMA_DECODE_PROTO(R, G, B, IA, {in += step;}) \
		break; \
	case ((N) << 1) + 1: \
		TVP_TLG6_DO_CHROMA_DECODE_PROTO2(R, G, B, IA, {in += step;}) \
		break;

void CTlg::TVPTLG6DecodeLineGeneric(const u32* prev_line, u32* current_line, u32 width,
                                    u32 start_block, u32 block_limit, const u8* filter_types,
                                    u32 skip_block_bytes, const u32* in, u32 initial_p,
                                    u32 odd_skip, u32 dir)
{
	// Chroma/luminosity decoding (this does reordering, color correlation filter, MED/AVG  at a time)
	u32 p, up;

	if (start_block)
	{
		prev_line += start_block * TVP_TLG6_W_BLOCK_SIZE;
		current_line  += start_block * TVP_TLG6_W_BLOCK_SIZE;
		p  = current_line[-1];
		up = prev_line[-1];
	}
	else
	{
		p = up = initial_p;
	}

	in += skip_block_bytes * start_block;
	const s32 step = (dir & 1) ? 1 : -1;

	for (u32 i = start_block; i < block_limit; i++)
	{
		s32 w = width - i * TVP_TLG6_W_BLOCK_SIZE;

		if (w > TVP_TLG6_W_BLOCK_SIZE)
			w = TVP_TLG6_W_BLOCK_SIZE;

		const s32 ww = w;

		if (step == -1) 
			in += ww - 1;

		if (i & 1) 
			in += odd_skip * ww;

		switch (filter_types[i])
		{
#define IA  static_cast<char>((*in >> 24) & 0xff)
#define IR  static_cast<char>((*in >> 16) & 0xff)
#define IG  static_cast<char>((*in >>  8) & 0xff)
#define IB  static_cast<char>((*in      ) & 0xff)
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
			in += skip_block_bytes - ww;
		else
			in += skip_block_bytes + 1;

		if (i & 1)
			in -= odd_skip * ww;
#undef IR
#undef IG
#undef IB
	}
}

void CTlg::TVPTLG6DecodeLine(const u32* prev_line, u32* current_line, u32 width,
                             u32 block_count, const u8* filter_types, u32 skip_block_bytes,
                             const u32* in, u32 initial_p, u32 odd_skip, u32 dir)
{
	TVPTLG6DecodeLineGeneric(prev_line, current_line, width, 0, block_count, filter_types, skip_block_bytes, in, initial_p, odd_skip, dir);
}

u32 CTlg::make_gt_mask(u32 a, u32 b)
{
	const u32 tmp2 = ~b;
	u32 tmp = ((a & tmp2) + (((a ^ tmp2) >> 1) & 0x7f7f7f7f) ) & 0x80808080;
	tmp = ((tmp >> 7) + 0x7f7f7f7f) ^ 0x7f7f7f7f;
	return tmp;
}

u32 CTlg::packed_bytes_add(u32 a, u32 b)
{
	const u32 tmp = (((a & b)<<1) + ((a ^ b) & 0xfefefefe)) & 0x01010100;
	return a + b - tmp;
}

u32 CTlg::med2(u32 a, u32 b, u32 c)
{
	// Do Median Edge Detector   thx, Mr. sugi  at    kirikiri.info
	const u32 aa_gt_bb = make_gt_mask(a, b);
	const u32 a_xor_b_and_aa_gt_bb = ((a ^ b) & aa_gt_bb);
	const u32 aa = a_xor_b_and_aa_gt_bb ^ a;
	const u32 bb = a_xor_b_and_aa_gt_bb ^ b;
	const u32 n = make_gt_mask(c, bb);
	const u32 nn = make_gt_mask(aa, c);
	const u32 m = ~(n | nn);

	return (n & aa) | (nn & bb) | ((bb & m) - (c & m) + (aa & m));
}

u32 CTlg::med(u32 a, u32 b, u32 c, u32 v)
{
	return packed_bytes_add(med2(a, b, c), v);
}

#define TLG6_AVG_PACKED(x, y) ((((x) & (y)) + ((((x) ^ (y)) & 0xfefefefe) >> 1)) +\
			(((x) ^ (y)) & 0x01010101))

u32 CTlg::avg(u32 a, u32 b, u32 c, u32 v)
{
	return packed_bytes_add(TLG6_AVG_PACKED(a, b), v);
}

void CTlg::InitTLG6Table()
{
	// Table which indicates first set bit position + 1.
	// This may be replaced by BSF (IA32 instruction).

	for (size_t i = 0; i < TVP_TLG6_LeadingZeroTable_SIZE; i++)
	{
		size_t cnt = 0;
		size_t j;

		for (j = 1; j != TVP_TLG6_LeadingZeroTable_SIZE && !(i & j);
			j <<= 1, cnt++);

		cnt++;

		if (j == TVP_TLG6_LeadingZeroTable_SIZE)
			cnt = 0;

		TVPTLG6LeadingZeroTable[i] = static_cast<u8>(cnt);
	}

	memset(TVPTLG6GolombBitLengthTable, 0, TVP_TLG6_GOLOMB_N_COUNT*2*128 * TVP_TLG6_GOLOMB_N_COUNT);
	for (size_t n = 0; n < TVP_TLG6_GOLOMB_N_COUNT; n++)
	{
		size_t a = 0;
		for (size_t i = 0; i < 9; i++)
		{
			for (s32 j = 0; j < TVPTLG6GolombCompressed[n][i]; j++)
			{
				TVPTLG6GolombBitLengthTable[a++][n] = static_cast<char>(i);
			}
		}
	}
}
