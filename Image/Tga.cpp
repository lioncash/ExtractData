#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Image.h"
#include "Tga.h"

/// Decoding
///
/// @param archive        Archive
/// @param src            TGA data
/// @param src_size       TGA data size
/// @param file_last_name End of the filename
///
bool CTga::Decode(CArcFile* archive, const u8* src, size_t src_size, const YCString& file_last_name)
{
	const STGAHeader* tga_header = reinterpret_cast<const STGAHeader*>(src);

	src += sizeof(STGAHeader);
	src_size -= sizeof(STGAHeader);

	// Decompression
	std::vector<u8> src2;

	switch (tga_header->btImageType)
	{
	case 9:
	case 10: // RLE Compression
		src2.resize(((tga_header->wWidth * (tga_header->btDepth >> 3) + 3) & 0xFFFFFFFC) * tga_header->wHeight);

		DecompRLE(src2.data(), src2.size(), src, src_size, tga_header->btDepth);

		src = src2.data();
		src_size = src2.size();
		break;
	}

	CImage image;

	if (tga_header->btDepth == 0)
	{
		image.Init(archive, tga_header->wWidth, tga_header->wHeight, 32, nullptr, 0, file_last_name);
		image.WriteReverse(src, src_size);
		image.Close();
	}
	else
	{
		image.Init(archive, tga_header->wWidth, tga_header->wHeight, tga_header->btDepth, nullptr, 0, file_last_name);
		image.Write(src, src_size);
		image.Close();
	}

	return true;
}

/// Decompression of compressed TGAs
///
/// @param dst      Storage location
/// @param dst_size Storage location size
/// @param src      Compressed TGA data
/// @param src_size Compressed TGA data size
///
bool CTga::Decomp(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	STGAHeader tga_header;
	std::memcpy(&tga_header, src, sizeof(STGAHeader));

	src += sizeof(STGAHeader);
	src_size -= sizeof(STGAHeader);

	// Decompression
	switch (tga_header.btDepth)
	{
	case 9:
	case 10: // RLE
		DecompRLE(dst, dst_size, src, src_size, tga_header.btDepth);
		break;

	default: // No Compression
		memcpy(dst, src, src_size);
		break;
	}

	return true;
}

/// RLE Decompression
///
/// @param dst      Storage location
/// @param dst_size Storage location size
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param bpp      Number of bits
///
bool CTga::DecompRLE(u8* dst, size_t dst_size, const u8* src, size_t src_size, u8 bpp)
{
	size_t    src_ptr = 0;
	size_t    dst_ptr = 0;
	const u32 byte_count = (bpp >> 3);

	while (src_ptr < src_size && dst_ptr < dst_size)
	{
		u32 length = src[src_ptr++];

		if (length & 0x80)
		{
			length = (length & 0x7F) + 1;

			for (u32 i = 0; i < length; i++)
			{
				memcpy(&dst[dst_ptr], &src[src_ptr], byte_count);

				dst_ptr += byte_count;
			}

			src_ptr += byte_count;
		}
		else
		{
			length++;

			length *= byte_count;

			memcpy(&dst[dst_ptr], &src[src_ptr], length);

			src_ptr += length;
			dst_ptr += length;
		}

		if (memcmp(&src[src_ptr + 8], "TRUEVISION-XFILE", 16) == 0)
		{
			// End
			break;
		}
	}

	return true;
}
