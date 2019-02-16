#include "StdAfx.h"
#include "Extract/cpz/PB3B.h"

#include "ArcFile.h"
#include "Extract/cpz/JBP1.h"
#include "Image.h"

/// Decoding
///
/// @param archive    Archive
/// @param src        Compressed data
/// @param src_size   Compressed data size
/// @param decryption_function Decoding function
///
/// @remark @a src is decoded first
///
bool CPB3B::Decode(CArcFile* archive, u8* src, size_t src_size, PB3B_DECRYPT decryption_function)
{
	// Decryption
	Decrypt(src, src_size);

	// Get image information
	const u16 type   = *reinterpret_cast<const u16*>(&src[28]);
	const s32 width  = *reinterpret_cast<const u16*>(&src[30]);
	const s32 height = *reinterpret_cast<const u16*>(&src[32]);
	const u16 bpp    = *reinterpret_cast<const u16*>(&src[34]);

	switch (type)
	{
	case 1:
		Decode1(archive, src, src_size, width, height, bpp);
		break;

	case 2:
	case 3:
		Decode3(archive, src, src_size, width, height, bpp);
		break;

	case 4:
		Decode4(archive, src, src_size, width, height, bpp);
		break;

	case 5:
		Decode5(archive, src, src_size, width, height, bpp);
		break;

	case 6:
		Decode6(archive, src, src_size, width, height, bpp, decryption_function);
		break;

	default: // Unknown
		archive->OpenFile();
		archive->WriteFile(src, src_size);
		break;
	}

	return true;
}

/// Decryption
///
/// @param target Decoded data
/// @param size   Decoded data size
///
void CPB3B::Decrypt(u8* target, size_t size)
{
	for (size_t i = 8; i < 52; i += 2)
	{
		*reinterpret_cast<u16*>(&target[i]) ^= *reinterpret_cast<u16*>(&target[size - 3]);
	}

	for (size_t i = 8, j = 0; i < 52; i++, j++)
	{
		target[i] -= target[size - 47 + j];
	}
}

/// Decode 1
///
/// @param archive  Archive
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
/// @param height   Height
/// @param bpp      Number of bits
///
bool CPB3B::Decode1(CArcFile* archive, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp)
{
	// Ensure output buffer
	const u32 dst_size = static_cast<u32>(width * height * (bpp >> 3));
	std::vector<u8> dst(dst_size);

	// Decompress
	Decomp1(dst.data(), dst.size(), src, src_size, width, height, bpp);

	// Output
	CImage image;
	image.Init(archive, width, height, bpp);
	image.WriteReverse(dst.data(), dst.size());

	return true;
}

/// Decode 3
///
/// @param archive  Archive
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
/// @param height   Height
/// @param bpp      Number of bits
///
bool CPB3B::Decode3(CArcFile* archive, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp)
{
	// Ensure output buffer
	const u32 dst_size = static_cast<u32>(width * height * (bpp >> 3));
	std::vector<u8> dst(dst_size);

	// Decompress
	Decomp3(dst.data(), dst.size(), src, src_size, width, height, bpp);

	// Output
	CImage image;
	image.Init(archive, width, height, bpp);
	image.WriteReverse(dst.data(), dst.size());

	return true;
}

/// Decode 4
///
/// @param archive  Archive
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
/// @param height   Height
/// @param bpp      Number of bits
///
bool CPB3B::Decode4(CArcFile* archive, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp)
{
	archive->OpenFile();
	archive->WriteFile(src, src_size);

	return true;
}

/// Decode 5
///
/// @param archive  Archive
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
/// @param height   Height
/// @param bpp      Number of bits
///
bool CPB3B::Decode5(CArcFile* archive, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp)
{
	// Ensure base image buffer
	const u32 base_size = static_cast<u32>(width * height * (bpp >> 3));
	std::vector<u8> base(base_size);

	// Decompress base image
	Decomp5(base.data(), base.size(), src, src_size, width, height, bpp);

	// Output base image
	CImage image;
	image.Init(archive, width, height, bpp);
	image.WriteReverse(base.data(), base.size());
	image.Close();

	return true;
}

/// Decode 6
///
/// @param archive          Archive
/// @param src              Compressed data
/// @param src_size         Compressed data size
/// @param width            Width
/// @param height           Height
/// @param bpp              Number of bits
/// @param decrypt_function Decryption function
///
bool CPB3B::Decode6(CArcFile* archive, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp, PB3B_DECRYPT decrypt_function)
{
	// Ensure output buffer
	const u32 dst_size = static_cast<u32>(width * height * (bpp >> 3));
	std::vector<u8> dst(dst_size);

	// Get base image file name
	u8 base_file_name_bytes[64];
	memcpy(base_file_name_bytes, src + 52, 32);

	// Decrypt base image filename
	static constexpr std::array<u32, 4> crypt{{
		0x9CF375A6, 0xA37869C5, 0x794FA53E, 0xC73AFE59
	}};

	for (size_t i = 0; i < 2; i++)
	{
		for (size_t j = 0; j < crypt.size(); j++)
		{
			reinterpret_cast<u32*>(base_file_name_bytes)[i * 4 + j] ^= crypt[j];
		}
	}
	base_file_name_bytes[32] = '\0';

	YCString base_file_name = reinterpret_cast<char*>(base_file_name_bytes);
	base_file_name += _T(".pb3");

	// プリミティブリンクのベース画像のファイル名が一部変になっているので修正
	if (base_file_name == _T("ast-si0a_100.pb3"))
	{
		base_file_name = _T("st-si0a_100a.pb3");
	}

	// Store base image
	const SFileInfo* base_file_info = archive->GetFileInfo(base_file_name, true);

	// Base image exists
	if (base_file_info != nullptr)
	{
		std::vector<u8> base_src(base_file_info->size_cmp);

		// Read base image
		archive->SeekHed(base_file_info->start);
		archive->Read(base_src.data(), base_src.size());

		// Decrypt base image
		decrypt_function(base_src.data(), base_src.size(), archive, *base_file_info);
		Decrypt(base_src.data(), base_src.size());

		// Decompress base image
		Decomp5(dst.data(), dst.size(), base_src.data(), base_src.size(), width, height, bpp);
	}

	// Decompress difference image
	Decomp6(dst.data(), dst.size(), src, src_size, width, height, bpp);

	// Output
	CImage image;
	image.Init(archive, width, height, bpp);
	image.WriteReverse(dst.data(), dst.size());

	return true;
}

/// Decompression 1
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
/// @param height   Height
/// @param bpp      Number of bits
///
bool CPB3B::Decomp1(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp)
{
	const u32 flags_offset = *reinterpret_cast<const u32*>(&src[44]);
	const u32 compressed_data_offset = *reinterpret_cast<const u32*>(&src[48]);
	const s32 block_width = *reinterpret_cast<const u32*>(&src[24]);
	const s32 block_height = *reinterpret_cast<const u32*>(&src[24]);
	const u16 byte_count = bpp >> 3;
	const s32 line = width * byte_count;

	// Ensure LZSS decompression buffer
	const u32 temp_size = width * height;
	std::vector<u8> temp(temp_size);

	// Decompress
	for (size_t color = 0; color < byte_count; color++)
	{
		const u8* src_ptr = &src[flags_offset];
		size_t work = byte_count * 4;
		for (size_t i = 0; i < color; i++)
		{
			work += reinterpret_cast<const u32*>(src_ptr)[i];
		}
		src_ptr += work;

		// Get the LZSS size after decompression
		const u32 lzss_decode_size = *reinterpret_cast<const u32*>(&src_ptr[8]);

		// Get pointer to flag data
		const u8* flags = src_ptr + *reinterpret_cast<const u32*>(&src_ptr[0]) + *reinterpret_cast<const u32*>(&src_ptr[4]) + 12;

		// Get pointer to decompressed data
		const u8* compressed_data = &src[compressed_data_offset];
		work = byte_count * 4;
		for (size_t i = 0; i < color; i++)
		{
			work += reinterpret_cast<const u32*>(compressed_data)[i];
		}
		compressed_data += work;

		// バッファ不足に対処
		// dwLZSSDecodeSizeで毎回バッファ確保すれば対処する必要ないけど、メモリ確保／破棄は最小限に抑えたい
		if (temp_size < lzss_decode_size)
		{
			temp.resize(lzss_decode_size);
		}

		// LZSS Decompression
		const u32 flags_size = 0xFFFFFFFF;    // Unknown size
		const u32 compressed_data_size = 0xFFFFFFFF; // Unknown size
		DecompLZSS(temp.data(), lzss_decode_size, flags, flags_size, compressed_data, compressed_data_size);

		// Decompress compressed blocks
		u8* dst_ptr = &dst[color];
		const s32 block_count_width = (width + (block_width - 1)) / block_width;
		const s32 block_count_height = (height + (block_height - 1)) / block_height;
		u8 code = 0x80;
		size_t temp_ptr = 0;
		size_t flags_ptr = 0;
		size_t compressed_data_ptr = 0;

		flags = src_ptr + 12;
		compressed_data = src_ptr + *reinterpret_cast<const u32*>(&src_ptr[0]) + 12;

		for (s32 y = 0, block_y = 0; block_y < block_count_height; y += block_height, block_y++)
		{
			u8* next_block_ptr = dst_ptr;
			const s32 max_block_height = ((y + block_height) > height) ? height - y : block_height;

			for (s32 x = 0, block_x = 0; block_x < block_count_width; x += block_width, block_x++)
			{
				if (code == 0)
				{
					// 8-block processing
					flags_ptr++;
					code = 0x80;
				}

				// Processing 1 block
				u8* block_ptr = next_block_ptr;
				const s32 max_block_width = ((x + block_width) > width) ? width - x : block_width;

				// Compressed
				if (flags[flags_ptr] & code)
				{
					for (s32 i = 0; i < max_block_height; i++)
					{
						for (s32 j = 0; j < max_block_width; j++)
						{
							block_ptr[j * byte_count] = compressed_data[compressed_data_ptr];
						}

						block_ptr += line;
					}

					compressed_data_ptr++;
				}
				else // Not compressed
				{
					for (s32 i = 0; i < max_block_height; i++)
					{
						for (s32 j = 0; j < max_block_width; j++)
						{
							block_ptr[j * byte_count] = temp[temp_ptr++];
						}

						block_ptr += line;
					}
				}

				// Point to the next block
				next_block_ptr += block_width * byte_count;
				code >>= 1;
			}

			dst_ptr += line * block_height;
		}
	}

	return true;
}

/// Decompression 3
///
/// @param dst       Destination
/// @param dst_size  Destination size
/// @param src       Compressed data
/// @param src_size  Compressed data size
/// @param width     Width
/// @param height    Height
/// @param bpp       Number of bits
///
bool CPB3B::Decomp3(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp)
{
	// Get alpha value
	const u8* alpha = src + *reinterpret_cast<const u32*>(&src[44]);
	const u32 alpha_size = *reinterpret_cast<const u32*>(&src[48]);

	// Decompress
	CJBP1 jbp1;
	jbp1.Decomp(dst, &src[52], bpp, alpha, alpha_size);

	return true;
}

/// Decompression 4
///
/// @param dst       Destination
/// @param dst_size  Destination size
/// @param src       Compressed data
/// @param src_size  Compressed data size
/// @param width     Width
/// @param height    Height
/// @param bpp       Number of bits
///
bool CPB3B::Decomp4(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp)
{
	return true;
}

/// Decompression 5
///
/// @param dst       Destination
/// @param dst_size  Destination size
/// @param src       Compressed data
/// @param src_size  Compressed data size
/// @param width     Width
/// @param height    Height
/// @param bpp       Number of bits
///
bool CPB3B::Decomp5(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp)
{
	// LZSS decompression buffer
	const size_t temp_size = static_cast<size_t>(width * height);
	std::array<std::vector<u8>, 4> temp;

	// Decompress base image
	for (size_t i = 0; i < temp.size(); i++)
	{
		// Ensure LZSS decompression buffer
		temp[i].resize(temp_size);

		// Get pointer to flag data
		const u8* flags = src + 84 + *reinterpret_cast<const u32*>(&src[i * 8 + 52]);

		// Get pointer to compressed data
		const u8* compresed_data = src + 84 + *reinterpret_cast<const u32*>(&src[i * 8 + 56]);

		// LZSS Decompression
		const u32 flags_size = 0xFFFFFFFF;    // Unknown size
		const u32 compressed_data_size = 0xFFFFFFFF; // Unknown size
		DecompLZSS(&temp[i][0], temp_size, flags, flags_size, compresed_data, compressed_data_size);
	}

	// Store
	dst[0] = temp[0][0];
	dst[1] = temp[1][0];
	dst[2] = temp[2][0];
	dst[3] = temp[3][0];

	for (size_t i = 1; i < temp_size; i++)
	{
		dst[i * 4 + 0] = temp[0][i] + dst[i * 4 - 4];
		dst[i * 4 + 1] = temp[1][i] + dst[i * 4 - 3];
		dst[i * 4 + 2] = temp[2][i] + dst[i * 4 - 2];
		dst[i * 4 + 3] = temp[3][i] + dst[i * 4 - 1];
	}

	return true;
}

/// Decompress 6
///
/// Remarks: Assumes that the base image is stored in pbtDst
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
/// @param height   Height
/// @param bpp      Number of bits
///
bool CPB3B::Decomp6(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width, s32 height, u16 bpp)
{
	// Ensure LZSS decompression buffer
	const u32 lzss_decode_size = *reinterpret_cast<const u32*>(&src[24]);
	std::vector<u8> temp(lzss_decode_size);

	// LZSS Decompression
	const u8* flags = src + 84;
	const u8* compressed_data = src + 84 + *reinterpret_cast<const u32*>(&src[44]);
	const u32 flags_size = 0xFFFFFFFF; // Unknown size
	const u32 compressed_data_size = 0xFFFFFFFF; // Unknown size
	DecompLZSS(temp.data(), lzss_decode_size, flags, flags_size, compressed_data, compressed_data_size);

	// Decompression
	const u16 byte_count = bpp >> 3;
	const s32 line = width * 4;
	const s32 block_width = 8;
	const s32 block_height = 8;
	const s32 block_count_width = (width + (block_width - 1)) / block_width;
	const s32 block_count_height = (height + (block_height - 1)) / block_height;
	size_t flags_ptr = 0;
	size_t compressed_data_ptr = 0;
	u8 code = 0x80;

	flags = &temp[8];
	compressed_data = &temp[*reinterpret_cast<u32*>(&temp[0]) + 8];

	u8* dst_ptr = dst;

	for (s32 y = 0, block_y = 0; block_y < block_count_height; y += block_height, block_y++)
	{
		u8* next_block_ptr = dst_ptr;
		const s32 max_block_height = ((y + block_height) > height) ? height - y : block_height;

		for (s32 x = 0, block_x = 0; block_x < block_count_width; x += block_width, block_x++)
		{
			if (code == 0)
			{
				// Processing 8 blocks
				code = 0x80;
				flags_ptr++;
			}

			// Process 1 block
			u8* block_ptr = next_block_ptr;
			const s32 max_block_width = ((x + block_width) > width) ? width - x : block_width;

			if ((flags[flags_ptr] & code) == 0)
			{
				// Difference
				for (s32 i = 0; i < max_block_height; i++)
				{
					for (s32 j = 0; j < max_block_width; j++)
					{
						memcpy(&block_ptr[j * 4], &compressed_data[compressed_data_ptr + j * 4], 4);
					}

					compressed_data_ptr += max_block_width * 4;
					block_ptr += line;
				}
			}

			// Point to the next block
			next_block_ptr += block_width * 4;
			code >>= 1;
		}

		dst_ptr += line * block_height;
	}

	return true;
}
