#include "StdAfx.h"
#include "Mzx.h"

void CMzx::Decompress(u8* dst, size_t length, const u8* src)
{
	std::array<u8, 128> ring_buffer{};
	u8 last1 = 0;
	u8 last2 = 0;

	const u8* psrc = src + 8;
	u8* pdst = dst;
	u32 ring_wpos = 0;
	int clear_count = 0;

	for (size_t i = 0; i < length; )
	{
		if (clear_count <= 0)
		{
			clear_count = 0x1000;
			last1 = 0;
			last2 = 0;
		}
		clear_count -= (*psrc & 3) == 2 ? 1 : (*psrc >> 2) + 1;

		switch (*psrc & 3)
		{
		case 0:
			for (int j = *psrc >> 2; j >= 0; j--)
			{
				*pdst++ = last1;
				*pdst++ = last2;
				i += 2;
			}

			psrc++;
			break;

		case 1:
		{
			int j = *psrc++ >> 2;
			int k = *psrc++;
			k = -k * 2 - 2;
				
			for ( ; j >= 0; j--)
			{
				*pdst = pdst[k]; pdst++;
				*pdst = pdst[k]; pdst++;
				i += 2;
			}

			last1 = pdst[-2];
			last2 = pdst[-1];
			break;
		}

		case 2:
			last1 = *pdst++ = ring_buffer[(*psrc >> 2) * 2];
			last2 = *pdst++ = ring_buffer[(*psrc >> 2) * 2 + 1];
			i += 2;
			psrc++;
			break;

		default:
			for (int j = *psrc++ >> 2; j >= 0; j--)
			{
				last1 = ring_buffer[ring_wpos++] = *pdst++ = *psrc++;
				last2 = ring_buffer[ring_wpos++] = *pdst++ = *psrc++;
				ring_wpos &= 0x7f;
				i += 2;
			}
			break;
		}
	}
}
