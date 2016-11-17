#include "stdafx.h"
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
void CPB::DecompLZSS(u8* dst, DWORD dst_size, const u8* flags, DWORD flags_size, const u8* src, DWORD src_size)
{
	// Initialize Dictionary
	constexpr DWORD dictionary_size = 2048;
	std::vector<BYTE> dictionary(dictionary_size);
	DWORD dictionary_ptr = 2014;

	// Decompress
	DWORD src_ptr = 0;
	DWORD flags_ptr = 0;
	DWORD dst_ptr = 0;
	BYTE  code = 0x80;

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
			const WORD work = *(WORD*)&src[src_ptr];

			DWORD back = work >> 5;
			DWORD length = (work & 0x1F) + 3;

			// Adjust so that the buffer is not exceeded
			if (dst_ptr + length > dst_size)
			{
				length = dst_size - dst_ptr;
			}

			// Dictionary Reference
			for (DWORD i = 0; i < length; i++)
			{
				dst[dst_ptr + i] = dictionary[dictionary_ptr++] = dictionary[back++];

				dictionary_ptr &= (dictionary_size - 1);
				back &= (dictionary_size - 1);
			}

			src_ptr += 2;
			dst_ptr += length;
		}
		else // Not compressed
		{
			dst[dst_ptr++] = dictionary[dictionary_ptr++] = src[src_ptr++];

			dictionary_ptr &= (dictionary_size - 1);
		}

		code >>= 1;
	}
}
