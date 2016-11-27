#include "StdAfx.h"
#include "Extract/Retouch.h"

#include "ArcFile.h"
#include "Arc/LZSS.h"
#include "Image.h"

/// Mounting
///
/// @param archive Archive
///
bool CRetouch::Mount(CArcFile* archive)
{
	if (MountGYU(archive))
		return true;

	return false;
}


/// GYU Mounting
///
/// @param archive Archive
///
bool CRetouch::MountGYU(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".gyu"))
		return false;

	if (memcmp(archive->GetHeader(), "GYU\x1A", 4) != 0)
		return false;

	return archive->Mount();
}

/// Decoding
///
/// @param archive Archive
///
bool CRetouch::Decode(CArcFile* archive)
{
	if (DecodeGYU(archive))
		return true;

	return false;
}

/// GYU Decoding
///
/// @param archive Archive
///
bool CRetouch::DecodeGYU(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("GYU"))
		return false;

	// Read header
	GYUHeader gyu_header;
	archive->Read(&gyu_header, sizeof(gyu_header));
	if (memcmp(gyu_header.identifier, "GYU\x1A", 4) != 0)
	{
		// File does not matter
		archive->SeekCur(-static_cast<s64>(sizeof(gyu_header)));
		return false;
	}

	// Read pallet
	u8 palette[1024] = {};
	const u32 palette_size = gyu_header.pallets * 4;
	if (palette_size > 0)
	{
		// Palette exists
		u8 black_palette[1024] = {};
		archive->Read(palette, palette_size);

		if (memcmp(palette, black_palette, palette_size) == 0)
		{
			// All pallets set to zero
			for (size_t i = 0, j = 0; i < palette_size; i += sizeof(u32), j++)
			{
				palette[i + 0] = static_cast<u8>(j);
				palette[i + 1] = static_cast<u8>(j);
				palette[i + 2] = static_cast<u8>(j);
				palette[i + 3] = 0;
			}
		}
	}

	// Read
	const u32 src_size = gyu_header.compressed_sizes[0];
	std::vector<u8> src(src_size);
	archive->Read(src.data(), src.size());

	// Ensure output buffers exist
	const u32 dst_size = ((gyu_header.width * (gyu_header.bpp >> 3) + 3) & 0xFFFFFFFC) * gyu_header.height;
	std::vector<u8> dst(dst_size);

	// Decrypt GYU
	DecryptGYU(src.data(), src.size(), gyu_header.key);

	// Decompress GYU
	DecompGYU(dst.data(), dst.size(), src.data(), src.size());

	// Output
	if (gyu_header.compressed_sizes[1] != 0)
	{
		// Alpha value exists

		// Read
		const u32 src_size2 = gyu_header.compressed_sizes[1];
		std::vector<u8> src2(src_size2);
		archive->Read(src2.data(), src2.size());

		// Ensure output buffer exists
		const u32 dst_size2 = ((gyu_header.width * (gyu_header.bpp >> 3) + 3) & 0xFFFFFFFC) * gyu_header.height;
		std::vector<u8> dst2(dst_size2);

		// Decompress GYU
		DecompGYU(dst2.data(), dst2.size(), src2.data(), src2.size());

		// Ensure output buffer exists
		const u32 dst_32bit_size = gyu_header.width * gyu_header.height * 4;
		std::vector<u8> dst_32bit(dst_32bit_size);

		// Into 32-bit by adding the alpha value
		for (long i = 0, y = 0; i < dst_32bit_size && y < gyu_header.height; y++)
		{
			const long present_height = ((gyu_header.width + 3) & 0xFFFFFFFC) * y;

			for (long x = 0; x < gyu_header.width; x++)
			{
				dst_32bit[i++] = palette[dst[present_height + x] * 4 + 0];
				dst_32bit[i++] = palette[dst[present_height + x] * 4 + 1];
				dst_32bit[i++] = palette[dst[present_height + x] * 4 + 2];

				if (dst2[present_height + x] < 0x10)
				{
					dst_32bit[i++] = dst2[present_height + x] * 16;
				}
				else
				{
					dst_32bit[i++] = 0xFF;
				}
			}
		}

		// Output
		CImage image;
		image.Init(archive, gyu_header.width, gyu_header.height, 32);
		image.Write(dst_32bit.data(), dst_32bit.size());
	}
	else
	{
		// Alpha value does not exist
		CImage image;
		image.Init(archive, gyu_header.width, gyu_header.height, gyu_header.bpp, palette, sizeof(palette));
		image.Write(dst.data(), dst.size());
	}

	return true;
}

/// GYU Decryption
///
/// @param src      Encrypted data
/// @param src_size Encrypted data size
/// @param key      Decryption key
///
bool CRetouch::DecryptGYU(u8* src, size_t src_size, u32 key)
{
	// Construct table
	u32 table[625 * 2];
	u32 work = key;
	u32 work2;

	for (size_t i = 0; i < 624; i++)
	{
		table[i] = work & 0xFFFF0000;
		work = work * 0x10DCD + 1;

		table[i] |= work >> 16;
		work = work * 0x10DCD + 1;
	}

	// Preparation for the first phase
	ULARGE_INTEGER astuliWork[8];

	astuliWork[4].LowPart = 0x9908B0DF;
	astuliWork[4].HighPart = 0x9908B0DF;

	astuliWork[5].LowPart = 1;
	astuliWork[5].HighPart = 1;

	astuliWork[6].QuadPart = astuliWork[5].QuadPart << 31;

	astuliWork[7].QuadPart = astuliWork[6].QuadPart;
	astuliWork[7].LowPart -= astuliWork[5].LowPart;
	astuliWork[7].HighPart -= astuliWork[5].HighPart;

	astuliWork[0].QuadPart = *reinterpret_cast<const u64*>(&table[0]) & astuliWork[6].QuadPart;
	astuliWork[1].QuadPart = *reinterpret_cast<const u64*>(&table[1]) & astuliWork[7].QuadPart;
	astuliWork[2].QuadPart = *reinterpret_cast<const u64*>(&table[397]);

	// First phase
	for (u32 i = 0, j = 397; i < 226 && j < 623; i += 2, j += 2)
	{
		astuliWork[0].QuadPart |= astuliWork[1].QuadPart;
		astuliWork[0].LowPart >>= 1;
		astuliWork[0].HighPart >>= 1;

		astuliWork[2].QuadPart ^= astuliWork[0].QuadPart;

		astuliWork[3].QuadPart = astuliWork[1].QuadPart & astuliWork[5].QuadPart;
		astuliWork[3].LowPart -= astuliWork[5].LowPart;
		astuliWork[3].HighPart -= astuliWork[5].HighPart;
		astuliWork[3].QuadPart = (~astuliWork[3].QuadPart & astuliWork[4].QuadPart) ^ astuliWork[2].QuadPart;

		astuliWork[0].QuadPart = *reinterpret_cast<const u64*>(&table[i + 2]) & astuliWork[6].QuadPart;
		astuliWork[1].QuadPart = *reinterpret_cast<const u64*>(&table[i + 3]) & astuliWork[7].QuadPart;
		astuliWork[2].QuadPart = *reinterpret_cast<const u64*>(&table[j + 2]);

		*reinterpret_cast<u64*>(&table[i]) = astuliWork[3].QuadPart;
	}

	work = table[226] & 0x80000000;
	work2 = table[227] & 0x7FFFFFFF;

	work = (((work | work2) >> 1) ^ table[623]) ^ 0x9908B0DF;
	work2 = ((work2 | 0xFFFFFFFE) + 1) & 0x9908B0DF;

	table[226] = work ^ work2;

	// Preparation for the second phase
	table[624] = table[0];

	astuliWork[0].QuadPart = *reinterpret_cast<const u64*>(&table[227]) & astuliWork[6].QuadPart;
	astuliWork[1].QuadPart = *reinterpret_cast<const u64*>(&table[228]) & astuliWork[7].QuadPart;
	astuliWork[2].QuadPart = *reinterpret_cast<const u64*>(&table[0]);

	// Second phase
	for (u32 i = 227, j = 0; i < 625 && j < 398; i += 2, j += 2)
	{
		astuliWork[0].QuadPart |= astuliWork[1].QuadPart;
		astuliWork[0].LowPart >>= 1;
		astuliWork[0].HighPart >>= 1;

		astuliWork[2].QuadPart ^= astuliWork[0].QuadPart;

		astuliWork[3].QuadPart = astuliWork[1].QuadPart & astuliWork[5].QuadPart;
		astuliWork[3].LowPart -= astuliWork[5].LowPart;
		astuliWork[3].HighPart -= astuliWork[5].HighPart;
		astuliWork[3].QuadPart = (~astuliWork[3].QuadPart & astuliWork[4].QuadPart) ^ astuliWork[2].QuadPart;

		astuliWork[0].QuadPart = *reinterpret_cast<const u64*>(&table[i + 2]) & astuliWork[6].QuadPart;
		astuliWork[1].QuadPart = *reinterpret_cast<const u64*>(&table[i + 3]) & astuliWork[7].QuadPart;
		astuliWork[2].QuadPart = *reinterpret_cast<const u64*>(&table[j + 2]);

		*reinterpret_cast<u64*>(&table[i]) = astuliWork[3].QuadPart;
	}

	// Preparation for the third phase
	astuliWork[6].LowPart = 0x9D2C5680;
	astuliWork[6].HighPart = 0x9D2C5680;

	astuliWork[7].LowPart = 0xEFC60000;
	astuliWork[7].HighPart = 0xEFC60000;

	astuliWork[0].QuadPart = *reinterpret_cast<const u64*>(&table[0]);
	astuliWork[1].QuadPart = *reinterpret_cast<const u64*>(&table[0]);
	astuliWork[2].QuadPart = *reinterpret_cast<const u64*>(&table[2]);

	astuliWork[0].LowPart >>= 11;
	astuliWork[0].HighPart >>= 11;
	astuliWork[0].QuadPart ^= *reinterpret_cast<const u64*>(&table[0]);

	// Third phase
	for (u32 i = 0, j = 625; i < 624 && j < 1249; i += 4, j += 4)
	{
		astuliWork[1].QuadPart = astuliWork[0].QuadPart;
		astuliWork[3].QuadPart = astuliWork[2].QuadPart;

		astuliWork[0].LowPart <<= 7;
		astuliWork[0].HighPart <<= 7;
		astuliWork[0].QuadPart &= astuliWork[6].QuadPart;
		astuliWork[0].QuadPart ^= astuliWork[1].QuadPart;

		astuliWork[2].LowPart >>= 11;
		astuliWork[2].HighPart >>= 11;
		astuliWork[2].QuadPart ^= astuliWork[3].QuadPart;

		astuliWork[3].QuadPart = astuliWork[2].QuadPart;
		astuliWork[1].QuadPart = astuliWork[0].QuadPart;

		astuliWork[2].LowPart <<= 7;
		astuliWork[2].HighPart <<= 7;
		astuliWork[2].QuadPart &= astuliWork[6].QuadPart;
		astuliWork[2].QuadPart ^= astuliWork[3].QuadPart;

		astuliWork[0].LowPart <<= 15;
		astuliWork[0].HighPart <<= 15;
		astuliWork[0].QuadPart &= astuliWork[7].QuadPart;
		astuliWork[0].QuadPart ^= astuliWork[1].QuadPart;

		astuliWork[3].QuadPart = astuliWork[2].QuadPart;
		astuliWork[1].QuadPart = astuliWork[0].QuadPart;

		astuliWork[2].LowPart <<= 15;
		astuliWork[2].HighPart <<= 15;
		astuliWork[2].QuadPart &= astuliWork[7].QuadPart;

		astuliWork[0].LowPart >>= 18;
		astuliWork[0].HighPart >>= 18;

		astuliWork[1].QuadPart ^= astuliWork[0].QuadPart;
		astuliWork[2].QuadPart ^= astuliWork[3].QuadPart;

		astuliWork[0].QuadPart = *reinterpret_cast<const s64*>(&table[i + 4]);
		astuliWork[3].QuadPart = astuliWork[2].QuadPart;

		*reinterpret_cast<s64*>(&table[j]) = astuliWork[1].QuadPart;

		// 

		astuliWork[2].LowPart >>= 18;
		astuliWork[2].HighPart >>= 18;

		astuliWork[3].QuadPart ^= astuliWork[2].QuadPart;

		astuliWork[1].QuadPart = astuliWork[0].QuadPart;
		astuliWork[2].QuadPart = *reinterpret_cast<const s64*>(&table[i + 6]);

		astuliWork[0].LowPart >>= 11;
		astuliWork[0].HighPart >>= 11;
		astuliWork[0].QuadPart ^= astuliWork[1].QuadPart;

		*reinterpret_cast<s64*>(&table[j + 2]) = astuliWork[3].QuadPart;
	}

	// Decoding

	for (size_t i = 0, j = 625; i < 10; i++)
	{
		work = table[j++] % src_size;
		work2 = table[j++] % src_size;

		std::swap(src[work], src[work2]);
	}

	return true;
}

/// GYU Decompression
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Compressed data
/// @param src_size Compressed data size
///
bool CRetouch::DecompGYU(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	// LZSS Decompression
	CLZSS lzss;
	lzss.Decomp(dst, dst_size, src, src_size, 4096, 4078, 3);

	return true;
}
