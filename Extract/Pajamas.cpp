#include "StdAfx.h"
#include "Extract/Pajamas.h"

#include "ArcFile.h"
#include "Image.h"

/// Mounting
///
/// @param archive Archive
///
bool CPajamas::Mount(CArcFile* archive)
{
	if (MountDat1(archive))
		return true;

	if (MountDat2(archive))
		return true;

	return false;
}

/// GAMEDAT PACK Mounting
///
/// @param archive Archive
///
bool CPajamas::MountDat1(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".dat") && archive->GetArcExten() != _T(".pak"))
		return false;

	if (memcmp(archive->GetHeader(), "GAMEDAT PACK", 12) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->SeekHed(12);
	archive->ReadU32(&num_files);

	// Get index size from file count
	const u32 index_size = num_files * 24;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Get index of the file information
	const u32 file_name_index_size = num_files << 4;
	const u8* file_info_index = &index[file_name_index_size];

	// Get offset (Required for correction when the starting address is zero-based)
	const u32 offset = 16 + index_size;

	u32 index_ptr = 0;
	for (u32 i = 0; i < num_files; i++)
	{
		// Get filename
		TCHAR file_name[16];
		memcpy(file_name, &index[index_ptr], 16);

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = *reinterpret_cast<const u32*>(&file_info_index[0]) + offset;
		file_info.size_cmp = *reinterpret_cast<const u32*>(&file_info_index[4]);
		file_info.size_org = file_info.size_cmp;
		file_info.end = file_info.start + file_info.size_cmp;

		archive->AddFileInfo(file_info);

		index_ptr += 16;
		file_info_index += 8;
	}

	return true;
}

// GAMEDAT PAC2 Mounting
///
/// @param archive Archive
///
bool CPajamas::MountDat2(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".dat"))
		return false;

	if (memcmp(archive->GetHeader(), "GAMEDAT PAC2", 12) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->SeekHed(12);
	archive->ReadU32(&num_files);

	// Get index size from file count
	const u32 index_size = num_files * 40;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Get index of the file information
	const u32 file_name_index_size = num_files << 5;
	const u8* file_info_index = &index[file_name_index_size];

	// Get offset (Required for correction when the starting address is zero-based)
	const u32 offset = 16 + index_size;

	size_t index_ptr = 0;
	for (u32 i = 0; i < num_files; i++)
	{
		// Get filename
		TCHAR file_name[32];
		memcpy(file_name, &index[index_ptr], 32);

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = *reinterpret_cast<const u32*>(&file_info_index[0]) + offset;
		file_info.size_cmp = *reinterpret_cast<const u32*>(&file_info_index[4]);
		file_info.size_org = file_info.size_cmp;
		file_info.end = file_info.start + file_info.size_cmp;

		archive->AddFileInfo(file_info);

		index_ptr += 32;
		file_info_index += 8;
	}

	return true;
}

/// Decoding
///
/// @param archive Archive
///
bool CPajamas::Decode(CArcFile* archive)
{
	if (DecodeEPA(archive))
		return true;

	return false;
}

/// EPA Decoding
///
/// @param archive Archive
///
bool CPajamas::DecodeEPA(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->format != _T("EPA"))
		return false;

	// Read header
	std::array<u8, 16> header;
	archive->Read(header.data(), header.size());

	// Difference flag, Number of colors, width, height
	const u8 diff_flag = header[3];
	u16 bpp = header[4];
	const s32 width = *reinterpret_cast<s32*>(&header[8]);
	const s32 height = *reinterpret_cast<s32*>(&header[12]);

	switch (bpp)
	{
	case 0:
		bpp = 8;
		break;

	case 1:
		bpp = 24;
		break;

	case 2:
		bpp = 32;
		break;

	case 4:
		bpp = 8;
		break;
	}

	switch (diff_flag)
	{
	case 1: // Usual
		break;

	case 2: // Difference
		archive->SeekCur(40);
		break;

	default: // Unknown
		archive->SeekHed(file_info->start);
		return false;
	}

	// Read palette
	std::array<u8, 768> pallet;
	if (bpp == 8)
	{
		archive->Read(pallet.data(), pallet.size());
	}

	// Read EPA data
	size_t src_size = file_info->size_cmp - 16;
	if (bpp == 8)
	{
		src_size -= pallet.size();
	}
	std::vector<u8> src(src_size);
	archive->Read(src.data(), src_size);

	// Secure area to store the BMP data
	const size_t dst_size = width * height * (bpp >> 3);
	std::vector<u8> dst(dst_size);

	// Decompress EPA
	DecompEPA(dst.data(), dst_size, src.data(), src_size, static_cast<u32>(width));

	// Output
	CImage image;
	image.Init(archive, width, height, bpp, pallet.data(), pallet.size());
	image.WriteCompoBGRAReverse(dst.data(), dst.size());

	return true;
}

/// EPA Decompression
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
///
void CPajamas::DecompEPA(u8* dst, size_t dst_size, const u8* src, size_t src_size, u32 width)
{
	size_t src_ptr = 0;
	size_t dst_ptr = 0;

	// Offset table
	const u32 offsets[16] =
	{
		0, 1, width, width + 1,
		2, width - 1, width << 1, 3,
		(width << 1) + 2, width + 2, (width << 1) + 1, (width << 1) - 1,
		(width << 1) - 2, width - 2, width * 3, 4
	};

	// Decompression
	while (src_ptr < src_size && dst_ptr < dst_size)
	{
		u8 code = src[src_ptr++];
		u32 length = code & 0x07;

		if (code & 0xF0)
		{
			if (code & 0x08)
			{
				length = (length << 8) + src[src_ptr++];
			}

			if (length != 0)
			{
				code >>= 4;

				const u32 back = dst_ptr - offsets[code];

				if (dst_ptr + length > dst_size)
				{
					// Exceeds output buffer
					length = dst_size - dst_ptr;
				}

				for (size_t i = 0; i < length; i++)
				{
					dst[dst_ptr + i] = dst[back + i];
				}

				dst_ptr += length;
			}
		}
		else if (code != 0)
		{
			memcpy(&dst[dst_ptr], &src[src_ptr], code);

			src_ptr += code;
			dst_ptr += code;
		}
	}
}
