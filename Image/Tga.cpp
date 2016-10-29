#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Image.h"
#include "Tga.h"

/// Decoding
///
/// @param pclArc            Archive
/// @param pvSrc             TGA data
/// @param dwSrcSize         TGA data size
/// @param rfclsFileLastName End of the filename
///
bool CTga::Decode(CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize, const YCString& rfclsFileLastName)
{
	const BYTE*       pbtSrc = reinterpret_cast<const BYTE*>(pvSrc);
	const STGAHeader* psttgahSrc = (STGAHeader*)pvSrc;

	pbtSrc += sizeof(STGAHeader);
	dwSrcSize -= sizeof(STGAHeader);

	// Decompression

	YCMemory<BYTE> clmbtSrc2;

	switch (psttgahSrc->btImageType)
	{
	case 9:
	case 10: // RLE Compression

		DWORD dwSrcSize2 = ((psttgahSrc->wWidth * (psttgahSrc->btDepth >> 3) + 3) & 0xFFFFFFFC) * psttgahSrc->wHeight;
		clmbtSrc2.resize(dwSrcSize2);

		DecompRLE(&clmbtSrc2[0], dwSrcSize2, pbtSrc, dwSrcSize, psttgahSrc->btDepth);

		pbtSrc = &clmbtSrc2[0];
		dwSrcSize = dwSrcSize2;
		break;
	}

	CImage clImage;

	if (psttgahSrc->btDepth == 0)
	{
		clImage.Init(pclArc, psttgahSrc->wWidth, psttgahSrc->wHeight, 32, nullptr, 0, rfclsFileLastName);
		clImage.WriteReverse(pbtSrc, dwSrcSize);
		clImage.Close();
	}
	else
	{
		clImage.Init(pclArc, psttgahSrc->wWidth, psttgahSrc->wHeight, psttgahSrc->btDepth, nullptr, 0, rfclsFileLastName);
		clImage.Write(pbtSrc, dwSrcSize);
		clImage.Close();
	}

	return true;
}

/// Decompression of compressed TGAs
///
/// @param pvDst     Storage location
/// @param dwDstSize Storage location size
/// @param pvSrc     Compressed TGA data
/// @param dwSrcSize Compressed TGA data size
///
bool CTga::Decomp(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize)
{
	BYTE*             pbtDst = reinterpret_cast<BYTE*>(pvDst);
	const BYTE*       pbtSrc = reinterpret_cast<const BYTE*>(pvSrc);

	STGAHeader tga_header;
	std::memcpy(&tga_header, pvSrc, sizeof(STGAHeader));

	pbtSrc += sizeof(STGAHeader);
	dwSrcSize -= sizeof(STGAHeader);

	// Decompression
	switch (tga_header.btDepth)
	{
	case 9:
	case 10: // RLE
		DecompRLE(pbtDst, dwDstSize, pbtSrc, dwSrcSize, tga_header.btDepth);
		break;

	default: // No Compression
		memcpy(pbtDst, pbtSrc, dwSrcSize);
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
