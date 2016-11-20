#include "StdAfx.h"
#include "../../ExtractBase.h"
#include "PB.h"

/// Decompress LZSS
///
/// @param dst        Destination
/// @param dst_size   Destination size
/// @param flags      Flag data
/// @param flags_size Flag data size
/// @param src        Compressed data
/// @param src_size   Compressed data size
///
void CPB::DecompLZSS(u8* dst, size_t dst_size, const u8* flags, size_t flags_size, const u8* src, size_t src_size)
{
	// Initialize Dictionary
	constexpr size_t dictionary_size = 2048;
	std::vector<u8> dictionary(dictionary_size);
	size_t dictionary_ptr = 2014;

	// Decompress
	size_t src_ptr = 0;
	size_t flags_ptr = 0;
	size_t dst_ptr = 0;
	u8 code = 0x80;

	while (src_ptr < src_size && dst_ptr < dst_size && flags_ptr < flags_size)
	{
		if (code == 0)
		{
			flags_ptr++;
			code = 0x80;
		}

		// Is Compressed
		if (flags[flags_ptr] & code)
		{
			const u16 work = *reinterpret_cast<const u16*>(&src[src_ptr]);

			size_t back = work >> 5;
			size_t length = (work & 0x1F) + 3;

			// Adjust so that the buffer is not exceeded
			if (dst_ptr + length > dst_size)
			{
				length = dst_size - dst_ptr;
			}

			// Dictionary Reference
			for (size_t i = 0; i < length; i++)
			{
				dst[dst_ptr + i] = dictionary[dictionary_ptr++] = dictionary[back++];

				dictionary_ptr &= dictionary_size - 1;
				back &= dictionary_size - 1;
			}

			src_ptr += 2;
			dst_ptr += length;
		}
		else // Not compressed
		{
			dst[dst_ptr++] = dictionary[dictionary_ptr++] = src[src_ptr++];

			dictionary_ptr &= dictionary_size - 1;
		}

		code >>= 1;
	}
}
