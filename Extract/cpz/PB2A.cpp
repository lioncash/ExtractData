#include "StdAfx.h"
#include "Extract/cpz/PB2A.h"

#include "ArcFile.h"
#include "Extract/cpz/JBP1.h"
#include "Image.h"

/// Decode
///
/// Remark: pvSrc can be used to decrypt the first part
///
/// @param archive  Archive
/// @param src      Compressed data
/// @param src_size Compressed data size
///
bool CPB2A::Decode(CArcFile* archive, u8* src, size_t src_size)
{
	// Decrypt
	Decrypt(src, src_size);

	// Get image information
	const u16 type    = *reinterpret_cast<u16*>(&src[16]);
	const long width  = *reinterpret_cast<u16*>(&src[18]);
	const long height = *reinterpret_cast<u16*>(&src[20]);
	const u16 bpp     = *reinterpret_cast<u16*>(&src[22]);

	// Decompression
	switch (type)
	{
	case 1:
		Decode1(archive, src, src_size, width, height, bpp);
		break;

	case 2:
		Decode2(archive, src, src_size, width, height, bpp);
		break;

	case 3:
	case 4:
		Decode4(archive, src, src_size, width, height, bpp);
		break;

	case 5:
		Decode5(archive, src, src_size, width, height, bpp);
		break;

	case 6:
		Decode6(archive, src, src_size, width, height, bpp);
		break;

	default: // Unknown
		archive->OpenFile();
		archive->WriteFile(src, src_size);
		break;
	}

	return true;
}

///  Decoding
///
/// @param target Data to be decoded
/// @param size   Data size
///
void CPB2A::Decrypt(u8* target, size_t size)
{
	for (size_t i = 8; i < 32; i += 2)
	{
		*reinterpret_cast<u16*>(&target[i]) ^= *reinterpret_cast<u16*>(&target[size - 3]);
	}

	for (size_t i = 8, j = 0; i < 32; i++, j++)
	{
		target[i] -= target[size - 27 + j];
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
bool CPB2A::Decode1(CArcFile* archive, const u8* src, size_t src_size, long width, long height, u16 bpp)
{
	// Ensure output buffer
	const u32 dst_size = static_cast<u32>(width * height * (bpp >> 3));
	std::vector<u8> dst(dst_size);

	// Decompression
	Decomp1(dst.data(), dst.size(), src, src_size, width, height, bpp);

	// Output
	CImage image;
	image.Init(archive, width, height, bpp);
	image.WriteReverse(dst.data(), dst.size());

	return true;
}

/// Decode 2
///
/// @param archive  Archive
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
/// @param height   Height
/// @param bpp      Number of bits
///
bool CPB2A::Decode2(CArcFile* archive, const u8* src, size_t src_size, long width, long height, u16 bpp)
{
	// Ensure output buffer
	const u32 dst_size = static_cast<u32>(width * height * (bpp >> 3));
	std::vector<u8> dst(dst_size);

	// Decompression
	Decomp2(dst.data(), dst.size(), src, src_size, width, height, bpp);

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
bool CPB2A::Decode4(CArcFile* archive, const u8* src, size_t src_size, long width, long height, u16 bpp)
{
	// Ensure output buffer
	const u32 dst_size = static_cast<u32>(width * height * (bpp >> 3));
	std::vector<u8> dst(dst_size);

	// Decompression
	Decomp4(dst.data(), dst.size(), src, src_size, width, height, bpp);

	// Output
	CImage image;
	image.Init(archive, width, height, bpp);
	image.WriteReverse(dst.data(), dst.size());

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
bool CPB2A::Decode5(CArcFile* archive, const u8* src, size_t src_size, long width, long height, u16 bpp)
{
	// Ensure base image buffer
	const u32 base_size = static_cast<u32>(width * height * 4);
	std::vector<u8> base(base_size);

	// Decompress base image
	Decomp5(base.data(), base.size(), src, src_size, width, height, bpp, nullptr, 0);

	// Output base image
	CImage image;
	image.Init(archive, width, height, 32);
	image.WriteReverse(base.data(), base.size());
	image.Close();

	// Get number of image files
	const u32 num_image_files = *reinterpret_cast<const u32*>(&src[8]);
	if (num_image_files == 1)
	{
		// Only the base image exists
		return true;
	}

	// Ensure image difference buffer
	const size_t dst_size = base.size();
	std::vector<u8> dst(dst_size);

	// Output image difference
	for (u32 frame_number = 1; frame_number < num_image_files; frame_number++)
	{
		// Decompress image difference
		Decomp5(dst.data(), dst.size(), src, src_size, width, height, bpp, base.data(), frame_number);

		// Output image difference
		TCHAR diff_name[256];
		_stprintf(diff_name, _T("_%02u"), frame_number - 1);
		image.Init(archive, width, height, 32, nullptr, 0, diff_name);
		image.WriteReverse(dst.data(), dst.size(), false);
		image.Close();
	}

	return true;
}

/// Decode 6
///
/// @param archive  Archive
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
/// @param height   Height
/// @param bpp      Number of bits
///
bool CPB2A::Decode6(CArcFile* archive, const u8* src, size_t src_size, long width, long height, u16 bpp)
{
	// Ensure base image buffer
	const u32 base_size = static_cast<u32>(width * height * 4);
	std::vector<u8> base(base_size);

	// Decompress base image
	Decomp6(base.data(), base.size(), src, src_size, width, height, 32, nullptr, 0);

	// Output base image
	CImage image;
	image.Init(archive, width, height, 32);
	image.WriteReverse(base.data(), base.size());
	image.Close();

	// Get number of image files
	const u32 num_image_files = *reinterpret_cast<const u32*>(&src[8]);
	if (num_image_files == 1)
	{
		// Only the base image exists
		return true;
	}

	// Ensure difference image buffer
	const size_t dst_size = base.size();
	std::vector<u8> dst(dst_size);

	// Output difference image 
	for (u32 frame_number = 1; frame_number < num_image_files; frame_number++)
	{
		// Decompress difference image
		Decomp6(dst.data(), dst.size(), src, src_size, width, height, 32, base.data(), frame_number);

		// Output difference image
		TCHAR diff_name[256];
		_stprintf(diff_name, _T("_%02u"), frame_number - 1);
		image.Init(archive, width, height, 32, nullptr, 0, diff_name);
		image.WriteReverse(dst.data(), dst.size(), false);
		image.Close();
	}

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
bool CPB2A::Decomp1(u8* dst, size_t dst_size, const u8* src, size_t src_size, long width, long height, u16 bpp)
{
	const u32  flags_offset = *reinterpret_cast<const u32*>(&src[24]);
	const u32  compressed_data_offset = *reinterpret_cast<const u32*>(&src[28]);
	const long block_width = 8;
	const long block_height = 8;
	const u16  byte_count = bpp >> 3;
	const long line = width * byte_count;

	// Ensure LZSS decompression buffer
	const u32 temp_size = width * height * byte_count;
	std::vector<u8> temp(temp_size);
	size_t temp_idx = 0;

	// LZSS Decompression
	const u8* flags = &src[flags_offset];
	const u8* compressed_data = &src[compressed_data_offset];
	const u32 flags_size = compressed_data_offset - flags_offset;
	const u32 compressed_data_size = src_size - compressed_data_offset;

	DecompLZSS(temp.data(), temp.size(), flags, flags_size, compressed_data, compressed_data_size);

	// Change color componentss
	const long block_count_width = ((width + (block_width - 1)) / block_width);
	const long block_count_height = ((height + (block_height - 1)) / block_height);

	for (u16 color = 0; color < byte_count; color++)
	{
		u8* dst_ptr = &dst[color];

		for (long y = 0, block_y = 0; block_y < block_count_height; y += block_height, block_y++)
		{
			u8* next_block_ptr = dst_ptr;
			const long max_block_height = ((y + block_height) > height) ? height - y : block_height;

			// Process the block in one column
			for (long x = 0, block_x = 0; block_x < block_count_width; x += block_width, block_x++)
			{
				u8* block_ptr = next_block_ptr;
				const long max_block_width = ((x + block_width) > width) ? width - x : block_width;

				// Process 1 block
				for (long i = 0; i < max_block_height; i++)
				{
					for (long j = 0; j < max_block_width; j++)
					{
						block_ptr[j * byte_count] = temp[temp_idx++];
					}

					block_ptr += line;
				}

				// Refer to the next block
				next_block_ptr += block_width * byte_count;
			}

			// Refers to the bottom block
			dst_ptr += line * block_height;
		}
	}

	return true;
}

/// Decompression 2
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
/// @param height   Height
/// @param bpp      Number of bits
///
bool CPB2A::Decomp2(u8* dst, size_t dst_size, const u8* src, size_t src_size, long width, long height, u16 bpp)
{
	const u32  flags_offset = *reinterpret_cast<const u32*>(&src[24]);
	const u32  compressed_data_offset = *reinterpret_cast<const u32*>(&src[28]);
	const long block_width = 8;
	const long block_height = 8;
	const u16  byte_count = bpp >> 3;
	const long line = width * byte_count;

	// Ensure LZSS decompression buffer
	const u32 temp_size = width * height;
	std::vector<u8> temp(temp_size);

	// Decompression
	for (u16 color = 0; color < byte_count; color++)
	{
		const u8* src_work_ptr = &src[flags_offset];
		u32 work = byte_count * 4;

		for (size_t i = 0; i < color; i++)
		{
			work += reinterpret_cast<const u32*>(src_work_ptr)[i];
		}

		src_work_ptr += work;

		// Get LZSS size after decompression
		const u32 lzss_decode_size = *reinterpret_cast<const u32*>(&src_work_ptr[8]);

		// Get pointer to data flag
		const u8* flags = src_work_ptr + *reinterpret_cast<const u32*>(&src_work_ptr[0]) + *reinterpret_cast<const u32*>(&src_work_ptr[4]) + 12;

		// Get pointer to compressed data 
		const u8* compressed_data = &src[compressed_data_offset];

		work = byte_count * 4;
		for (size_t i = 0; i < color; i++)
		{
			work += reinterpret_cast<const u32*>(compressed_data)[i];
		}
		compressed_data += work;

		// Handle insufficient buffer space
		// Do not need to gurantee the size of dwLZSSDecodeSize anymore, this minimizes allocation
		if (temp_size < lzss_decode_size)
		{
			temp.resize(lzss_decode_size);
		}

		// LZSS Decompression
		const u32 flags_size = 0xFFFFFFFF; // Unknown Size
		const u32 compressed_data_size = 0xFFFFFFFF; // Unknown Size
		DecompLZSS(temp.data(), lzss_decode_size , flags, flags_size, compressed_data, compressed_data_size);

		// Decompress compressed blocks
		const long block_count_width = ((width + (block_width - 1)) / block_width);
		const long block_count_height = ((height + (block_height - 1)) / block_height);
		u8 code = 0x80;
		size_t temp_idx = 0;
		size_t flags_idx = 0;
		size_t compressed_data_idx = 0;

		flags = src_work_ptr + 12;
		compressed_data = src_work_ptr + *reinterpret_cast<const u32*>(&src_work_ptr[0]) + 12;

		u8* dst_ptr = &dst[color];

		for (long y = 0, block_y = 0; block_y < block_count_height; y += block_height, block_y++)
		{
			u8* next_block_ptr = dst_ptr;
			const long  max_block_height = ((y + block_height) > height) ? height - y : block_height;

			for (long x = 0, block_x = 0; block_x < block_count_width; x += block_width, block_x++)
			{
				if (code == 0)
				{
					// Processed 8 blocks
					flags_idx++;
					code = 0x80;
				}

				// Process 1 block
				u8* block_ptr = next_block_ptr;
				const long max_block_width = ((x + block_width) > width) ? width - x : block_width;

				// Compressed
				if (flags[flags_idx] & code)
				{
					for (long i = 0; i < max_block_height; i++)
					{
						for (long j = 0; j < max_block_width; j++)
						{
							block_ptr[j * byte_count] = compressed_data[compressed_data_idx];
						}

						block_ptr += line;
					}

					compressed_data_idx++;
				}
				else // Not compressed
				{
					for (long i = 0; i < max_block_height; i++)
					{
						for (long j = 0; j < max_block_width; j++)
						{
							block_ptr[j * byte_count] = temp[temp_idx++];
						}

						block_ptr += line;
					}
				}

				// Refer to next block
				next_block_ptr += block_width * byte_count;
				code >>= 1;
			}

			dst_ptr += line * block_height;
		}
	}

	return true;
}

/// Decompression 4
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
/// @param height   Height
/// @param bpp      Number of bits
///
bool CPB2A::Decomp4(u8* dst, size_t dst_size, const u8* src, size_t src_size, long width, long height, u16 bpp)
{
	// Get alpha value
	const u8* alpha = src + *reinterpret_cast<const u32*>(&src[24]);
	const u32 alpha_size = *reinterpret_cast<const u32*>(&src[28]);

	// Decompress
	CJBP1 jbp1;
	jbp1.Decomp(dst, &src[32], bpp, alpha, alpha_size);

	return true;
}

/// Decompression 5
///
/// Remark: if pbtBast is null, then difference compositing is not performed
///
/// @param dst          Destination
/// @param dst_size     Destination size
/// @param src          Compressed data
/// @param src_size     Compressed data size
/// @param width        Width
/// @param height       Height
/// @param bpp          Number of bits
/// @param base         Base image
/// @param frame_number Frame number
///
bool CPB2A::Decomp5(
	u8*       dst,
	size_t    dst_size,
	const u8* src,
	size_t    src_size,
	long      width,
	long      height,
	u16       bpp,
	const u8* base,
	u32       frame_number
	)
{
	// Decompression

	// Base image
	if (frame_number == 0)
	{
		// Buffer allocation for LZSS extraction
		const u32 temp_size = static_cast<u32>(width * height);
		std::vector<u8> temp[4];

		// LZSS Decompression
		for (size_t i = 0; i < 4; i++)
		{
			// Ensure the buffer can hold the LZSS data for extraction
			temp[i].resize(temp_size);

			// LZSS Decompression
			const u8* flags = src + 64 + *reinterpret_cast<const u32*>(&src[i * 8 + 32]);
			const u8* compressed_data = src + 64 + *reinterpret_cast<const u32*>(&src[i * 8 + 36]);
			const u32 flags_size = 0xFFFFFFFF;    // Unknown
			const u32 compressed_data_size = 0xFFFFFFFF; // Unknown

			DecompLZSS(temp[i].data(), temp_size, flags, flags_size, compressed_data, compressed_data_size);
		}

		for (size_t i = 0; i < temp_size; i++)
		{
			dst[i * 4 + 0] = temp[0][i];
			dst[i * 4 + 1] = temp[1][i];
			dst[i * 4 + 2] = temp[2][i];
			dst[i * 4 + 3] = temp[3][i];
		}

		// Decryption
		for (size_t i = 0; i < temp_size; i++)
		{
			const u8 data1 = dst[i * 4 + 2] ^ dst[i * 4 + 3];
			const u8 data2 = dst[i * 4 + 1] ^ data1;

			dst[i * 4 + 2] = data1;
			dst[i * 4 + 1] = data2;
			dst[i * 4 + 0] ^= data2;
		}
	}
	else // Difference image
	{
		const u8* diff_src = src + *reinterpret_cast<const u32*>(&src[12]);
		for (u32 i = 1; i < frame_number; i++)
		{
			diff_src += *reinterpret_cast<const u32*>(&diff_src[0]);
		}

		// Buffer allocation for LZSS extraction
		const u32 lzss_decode_size = *reinterpret_cast<const u32*>(&diff_src[12]);
		std::vector<u8> temp(lzss_decode_size);

		// LZSS Decompression
		const u8* flags = diff_src + 16;
		const u8* compressed_data = diff_src + 16 + *reinterpret_cast<const u32*>(&diff_src[4]);
		const u32 flags_size = 0xFFFFFFFF; // Unknown
		const u32 compressed_data_size = 0xFFFFFFFF; // Unknown
		DecompLZSS(temp.data(), temp.size(), flags, flags_size, compressed_data, compressed_data_size);

		// Copy base image
		if (base != nullptr)
		{
			memcpy(dst, base, dst_size);
		}
		else
		{
			memset(dst, 0xFF, dst_size);
		}

		// Decompression
		const long line = width * 4;
		const long block_width = 8;
		const long block_height = 8;
		const long block_count_width = ((width + (block_width - 1)) / block_width);
		const long block_count_height = ((height + (block_height - 1)) / block_height);
		size_t flags_idx = 0;
		size_t compressed_data_idx = 0;
		u8 code = 0x80;

		flags = &temp[8];
		compressed_data = &temp[*reinterpret_cast<u32*>(&temp[0]) + 8];

		u8* dst_ptr = dst;

		for (long y = 0, block_y = 0; block_y < block_count_height; y += block_height, block_y++)
		{
			// Process the block of one column
			u8* next_block_ptr = dst_ptr;
			const long max_block_height = ((y + block_height) > height) ? height - y : block_height;

			for (long x = 0, block_x = 0; block_x < block_count_width; x += block_width, block_x++)
			{
				if (code == 0)
				{
					// Processed 8 blocks
					code = 0x80;
					flags_idx++;
				}

				// Process 1 block
				u8* block_ptr = next_block_ptr;
				const long max_block_width = ((x + block_width) > width) ? width - x : block_width;

				if ((flags[flags_idx] & code) == 0)
				{
					// Difference

					for (long i = 0; i < max_block_height; i++)
					{
						for (long j = 0; j < max_block_width; j++)
						{
							memcpy(&block_ptr[j * 4], &compressed_data[compressed_data_idx + j * 3], 3);

							block_ptr[j * 4 + 3] = 0xFF;
						}

						compressed_data_idx += 3;
						block_ptr += line;
					}
				}

				// Refer to next block
				next_block_ptr += block_width * 4;
				code >>= 1;
			}

			dst_ptr += line * block_height;
		}
	}

	return true;
}

/// Decompress 6
///
/// @param dst          Destination
/// @param dst_size     Destination size
/// @param src          Compressed data
/// @param src_size     Compressed data size
/// @param width        Width
/// @param height       Height
/// @param bpp          Number of bits
/// @param base         Base image
/// @param frame_number Frame number
///
/// @remark If @a base is NULL, then difference image compositing is not performed
///
bool CPB2A::Decomp6(
	u8*       dst,
	size_t    dst_size,
	const u8* src,
	size_t    src_size,
	long      width,
	long      height,
	u16       bpp,
	const u8* base,
	u32       frame_number
	)
{
	// Decompression

	// Base image
	if (frame_number == 0)
	{
		const u8* src_base = src + 36 + ((strlen(reinterpret_cast<const char*>(src) + 36) + 4) & 0xFFFFFFFC);

		// Buffer allocation for LZSS extraction
		const u32 temp_size = static_cast<u32>(width * height);
		std::array<std::vector<u8>, 4> temp;

		// LZSS Decompression
		for (size_t i = 0; i < temp.size(); i++)
		{
			// Buffer allocation for LZSS extraction
			temp[i].resize(temp_size);

			// LZSS Decompression
			const u8* flags = src_base + 32 + *reinterpret_cast<const u32*>(&src_base[i * 8]);
			const u8* compressed_data = src_base + 32 + *reinterpret_cast<const u32*>(&src_base[i * 8 + 4]);
			const u32 flags_size = 0xFFFFFFFF;    // Unknown
			const u32 compressed_data_size = 0xFFFFFFFF; // Unknown

			DecompLZSS(&temp[i][0], temp_size, flags, flags_size, compressed_data, compressed_data_size);
		}

		for (size_t i = 0; i < temp_size; i++)
		{
			dst[i * 4 + 0] = temp[0][i];
			dst[i * 4 + 1] = temp[1][i];
			dst[i * 4 + 2] = temp[2][i];
			dst[i * 4 + 3] = temp[3][i];
		}

		// Decryption
		for (size_t i = 0; i < temp_size; i++)
		{
			const u8 data1 = dst[i * 4 + 2] ^ dst[i * 4 + 3];
			const u8 data2 = dst[i * 4 + 1] ^ data1;

			dst[i * 4 + 2] = data1;
			dst[i * 4 + 1] = data2;
			dst[i * 4 + 0] ^= data2;
		}
	}
	else // Difference image
	{
		const u8* diff_src = src + *reinterpret_cast<const u32*>(&src[32]) + 32;
		for (u32 i = 1; i < frame_number; i++)
		{
			diff_src += *reinterpret_cast<const u32*>(&diff_src[0]);
		}

		// Get difference image file name
		char diff_name[256];
		const size_t name_len = ((strlen(reinterpret_cast<const char*>(diff_src) + 4) + 4) & 0xFFFFFFFC) + 4;
		strcpy(diff_name, reinterpret_cast<const char*>(diff_src) + 4);

		// Buffer allocation for LZSS extraction
		const u32 lzss_decode_size = *reinterpret_cast<const u32*>(&diff_src[name_len + 8]);
		std::vector<u8> temp(lzss_decode_size);

		// LZSS Decompression
		const u8* flags = diff_src + 16 + name_len;
		const u8* compressed_data = diff_src + 16 + name_len + *reinterpret_cast<const u32*>(&diff_src[name_len]);
		const u32 flags_size = 0xFFFFFFFF; // Unknown
		const u32 compressed_data_size = 0xFFFFFFFF; // Unknown
		DecompLZSS(temp.data(), temp.size(), flags, flags_size, compressed_data, compressed_data_size);

		// Copy base image
		if (base != nullptr)
		{
			memcpy(dst, base, dst_size);
		}
		else
		{
			memset(dst, 0xFF, dst_size);
		}

		// Decompression
		const u16  byte_count = bpp >> 3;
		const long line = width * 4;
		const long block_width = 8;
		const long block_height = 8;
		const long block_count_width = ((width + (block_width - 1)) / block_width);
		const long block_count_height = ((height + (block_height - 1)) / block_height);
		size_t flags_idx = 0;
		size_t compressed_data_idx = 0;
		u8 code = 0x80;

		flags = &temp[8];
		compressed_data = &temp[*reinterpret_cast<u32*>(&temp[0]) + 8];

		u8* dst_ptr = dst;

		for (long y = 0, block_y = 0; block_y < block_count_height; y += block_height, block_y++)
		{
			u8* next_block_ptr = dst_ptr;
			const long max_block_height = ((y + block_height) > height) ? height - y : block_height;

			// Process 1 column
			for (long x = 0, block_x = 0; block_x < block_count_width; x += block_width, block_x++)
			{
				if (code == 0)
				{
					// 8 blocks processed
					code = 0x80;
					flags_idx++;
				}

				// Process 1 block
				u8* block_ptr = next_block_ptr;
				const long max_block_width = ((x + block_width) > width) ? width - x : block_width;

				if ((flags[flags_idx] & code) == 0)
				{
					// Difference
					for (long i = 0; i < max_block_height; i++)
					{
						for (long j = 0; j < max_block_width; j++)
						{
							memcpy(&block_ptr[j * 4], &compressed_data[compressed_data_idx + j * 4], 4);
						}

						compressed_data_idx += max_block_width * 4;
						block_ptr += line;
					}
				}

				// Refer to the next block
				next_block_ptr += block_width * 4;
				code >>= 1;
			}

			dst_ptr += line * block_height;
		}
	}

	return true;
}
