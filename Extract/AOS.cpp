#include "StdAfx.h"
#include "Extract/AOS.h"

#include "ArcFile.h"
#include "Common.h"
#include "Image.h"

/// Mounting
///
/// @param archive Archive
///
bool CAOS::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".aos"))
		return false;

	// Unknown 4 bytes
	archive->SeekHed(4);

	// Get offset
	u32 offset;
	archive->ReadU32(&offset);

	// Get index size
	u32 index_size;
	archive->ReadU32(&index_size);

	// Get archive filename
	char archive_name[261];
	archive->Read(archive_name, 261);
	if (archive->GetArcName() != archive_name)
	{
		// Archive filename is different

		archive->SeekHed();
		return false;
	}

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Get file info
	for (size_t i = 0; i < index.size(); i += 40)
	{
		SFileInfo file_info;
		file_info.name.Copy(reinterpret_cast<const char*>(&index[i]), 32);
		file_info.start = *reinterpret_cast<const u32*>(&index[i + 32]) + offset;
		file_info.size_cmp = *reinterpret_cast<const u32*>(&index[i + 36]);
		file_info.size_org = file_info.size_cmp;
		file_info.end = file_info.start + file_info.size_cmp;

		archive->AddFileInfo(file_info);
	}

	return true;
}

/// General decoding
///
/// @param archive Archive
///
bool CAOS::Decode(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".aos"))
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format == _T("ABM"))
		return DecodeABM(archive);

	// Image mask
	if (file_info->format == _T("MSK"))
		return DecodeMask(archive);

	// Script
	if (file_info->format == _T("SCR"))
		return DecodeScript(archive);

	return false;
}


/// ABM Decoding
///
/// @param archive Archive
///
bool CAOS::DecodeABM(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Read data
	std::vector<u8> src(file_info->size_cmp);
	archive->Read(src.data(), src.size());

	// Get bitmap header
	const auto* bitmap_file_header = (const BITMAPFILEHEADER*) &src[0];
	const auto* bitmap_info_header = (const BITMAPINFOHEADER*) &src[14];

	CImage          image;
	std::vector<u8> dst;
	u32             dst_size;

	switch (bitmap_info_header->biBitCount)
	{
	case 1: // Multi-frame
	{
		YCString last_name;
		const u32 frames = *(const u32*)&src[58];
		const u32 data_offset = *(const u32*)&src[66];

		dst_size = bitmap_info_header->biWidth * bitmap_info_header->biHeight * 4;
		dst.resize(dst_size);

		// Multiple files
		if (frames >= 2)
		{
			last_name.Format(_T("_000"));
		}

		// Decompression
		u32 src_ptr = data_offset;
		for (size_t i = 0; i < dst.size(); i += 4)
		{
			dst[i + 0] = src[src_ptr++];
			dst[i + 1] = src[src_ptr++];
			dst[i + 2] = src[src_ptr++];
			dst[i + 3] = 0xFF;
		}

		// Output
		image.Init(archive, bitmap_info_header->biWidth, bitmap_info_header->biHeight, 32, nullptr, 0, last_name);
		image.WriteReverse(dst.data(), dst.size());
		image.Close();

		for (u32 i = 1; i < frames; i++)
		{
			const u32 frame_offset = *(const u32*)&src[70 + (i - 1) * 4];
			last_name.Format(_T("_%03u"), i);

			// Decompression
			std::fill(dst.begin(), dst.end(), 0);
			DecompABM(dst.data(), dst.size(), &src[frame_offset], src.size() - frame_offset);

			// Output
			image.Init(archive, bitmap_info_header->biWidth, bitmap_info_header->biHeight, 32, nullptr, 0, last_name);
			image.WriteReverse(dst.data(), dst.size(), false);
			image.Close();
		}
		break;
	}

	case 32: // 32bit
		dst_size = bitmap_info_header->biWidth * bitmap_info_header->biHeight * 4;
		dst.resize(dst_size);

		// Decompression
		DecompABM(dst.data(), dst.size(), &src[54], src.size() - 54);

		// Output
		image.Init(archive, bitmap_info_header->biWidth, bitmap_info_header->biHeight, bitmap_info_header->biBitCount);
		image.WriteReverse(dst.data(), dst.size());
		image.Close();
		break;

	default: // Other
		archive->OpenFile();
		archive->WriteFile(src.data(), src.size());
		archive->CloseFile();
	}

	return true;
}

/// Decode Image Mask
///
/// @param archive Archive
///
bool CAOS::DecodeMask(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Read Data
	std::vector<u8> src(file_info->size_cmp);
	archive->Read(src.data(), src.size());

	// Output
	CImage image;
	image.Init(archive, src.data());
	image.Write(src.size());
	image.Close();

	return true;
}

/// Decode Script
///
/// @param archive Archive
///
bool CAOS::DecodeScript(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Read compressed data
	const u32 src_size = file_info->size_cmp;
	std::vector<u8> src(src_size);
	archive->Read(src.data(), src.size());

	// Buffer allocation for extraction
	const u32 dst_size = *(const u32*) &src[0];
	std::vector<u8> dst(dst_size);

	// Decompression
	DecompScript(dst.data(), dst.size(), &src[4], src_size - 4);

	// Output
	archive->OpenScriptFile();
	archive->WriteFile(dst.data(), dst.size(), src_size);
	archive->CloseFile();

	return true;
}

/// ABM Decompression
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Compressed data
/// @param src_size Compressed data size
///
bool CAOS::DecompABM(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	size_t src_ptr = 0;
	size_t dst_ptr = 0;
	u32 alpha_count = 0;

	while (dst_ptr < dst_size)
	{
		const u8 current_src = src[src_ptr++];

		switch (current_src)
		{
		case 0: // Is 0x00
		{
			const u32 length = src[src_ptr++];

			for (size_t i = 0; i < length; i++)
			{
				dst[dst_ptr++] = 0;

				alpha_count++;

				if (alpha_count == 3)
				{
					dst[dst_ptr++] = 0;
					alpha_count = 0;
				}
			}

			break;
		}

		case 255: // Is 0xFF
		{
			const u32 length = src[src_ptr++];

			for (size_t i = 0; i < length; i++)
			{
				dst[dst_ptr++] = src[src_ptr++];

				alpha_count++;

				if (alpha_count == 3)
				{
					dst[dst_ptr++] = 0xFF;
					alpha_count = 0;
				}
			}
			break;
		}

		default: // Other
			dst[dst_ptr++] = src[src_ptr++];
			alpha_count++;

			if (alpha_count == 3)
			{
				dst[dst_ptr++] = current_src;
				alpha_count = 0;
			}
		}
	}

	return true;
}

/// Decompress Script
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Compressed data
/// @param src_size Compressed data size
///
bool CAOS::DecompScript(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	// Construct huffman table
	u32 bit0_table[511] = {};
	u32 bit1_table[511] = {};
	size_t src_ptr = 0;
	size_t table_ptr = 256;
	u32 current_src = 0;
	u32 bit_shift = 0;

	table_ptr = CreateHuffmanTable(bit0_table, bit1_table, src, &src_ptr, &table_ptr, &current_src, &bit_shift);

	// Decompress
	DecompHuffman(dst, dst_size, bit0_table, bit1_table, &src[src_ptr], table_ptr, current_src, bit_shift);

	return true;
}

/// Construct Huffman Table
///
/// @param bit0_table  bit0 table
/// @param bit1_table  bit1 table
/// @param src         Compressed data
/// @param src_ptr     Compressed data pointer
/// @param table_ptr   Table pointer
/// @param current_src Current data
/// @param bit_shift   Bit shift
///
u32 CAOS::CreateHuffmanTable(u32* bit0_table, u32* bit1_table, const u8* src, size_t* src_ptr, size_t* table_ptr, u32* current_src, u32* bit_shift)
{
	u32 return_value = 0;

	if (*bit_shift == 0)
	{
		// Read 8-bits

		*current_src = src[(*src_ptr)++];
		*bit_shift = 8;
	}

	*bit_shift -= 1;

	// Bit 1
	if ((*current_src >> *bit_shift) & 1)
	{
		const u32 table_ptr_value = *table_ptr;

		*table_ptr += 1;

		if (table_ptr_value < 511)
		{
			bit0_table[table_ptr_value] = CreateHuffmanTable(bit0_table, bit1_table, src, src_ptr, table_ptr, current_src, bit_shift);
			bit1_table[table_ptr_value] = CreateHuffmanTable(bit0_table, bit1_table, src, src_ptr, table_ptr, current_src, bit_shift);

			return_value = table_ptr_value;
		}
	}
	else // Bit 0
	{
		u32 bit_shift_temp = 8;
		u32 result = 0;

		while (bit_shift_temp > *bit_shift)
		{
			const u32 work = ((1 << *bit_shift) - 1) & *current_src;

			bit_shift_temp -= *bit_shift;

			*current_src = src[(*src_ptr)++];

			result |= work << bit_shift_temp;

			*bit_shift = 8;
		}
		*bit_shift -= bit_shift_temp;

		const u32 mask = (1 << bit_shift_temp) - 1;
		return_value = ((*current_src >> *bit_shift) & mask) | result;
	}

	return return_value;
}

/// Huffman Decompression
///
/// @param dst         Destination
/// @param dst_size    Destination size
/// @param bit0_table  bit0 table
/// @param bit1_table  bit1 table
/// @param src         Compressed data
/// @param root        Table position reference
/// @param current_src Current data
/// @param bit_shift   Bit shift
///
bool CAOS::DecompHuffman(u8* dst, size_t dst_size, const u32* bit0_table, const u32* bit1_table, const u8* src, size_t root, u32 current_src, u32 bit_shift)
{
	if (dst_size <= 0)
		return false;

	size_t src_ptr = 0;
	size_t dst_ptr = 0;

	while (dst_ptr < dst_size)
	{
		size_t table_ptr = root;

		while (table_ptr >= 256)
		{
			if (bit_shift == 0)
			{
				// Read 8-bits
				current_src = src[src_ptr++];
				bit_shift = 8;
			}

			bit_shift -= 1;

			// bit 1
			if ((current_src >> bit_shift) & 1)
			{
				table_ptr = bit1_table[table_ptr];
			}
			else // bit 0
			{
				table_ptr = bit0_table[table_ptr];
			}
		}

		dst[dst_ptr++] = static_cast<u8>(table_ptr);
	}

	return true;
}
