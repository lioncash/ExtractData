#include "StdAfx.h"
#include "Extract/Will.h"

#include "ArcFile.h"
#include "Common.h"
#include "Image.h"
#include "UI/ProgressBar.h"

namespace
{
bool HasValidHeaderID(const CArcFile* archive)
{
	static constexpr std::array<const char*, 7> header_ids{{
		"OGG", "WSC", "ANM", "MSK", "WIP", "TBL", "SCR"
	}};

	const u8* const archive_id_start = &archive->GetHeader()[4];

	return std::any_of(header_ids.begin(), header_ids.end(), [archive_id_start](const char* id) {
		return std::memcmp(archive_id_start, id, 4) == 0;
	});
}
} // Anonymous namespace

/// Mounting
bool CWill::Mount(CArcFile* archive)
{
	if (lstrcmpi(archive->GetArcExten(), _T(".arc")) != 0)
		return false;

	if (!HasValidHeaderID(archive))
		return false;

	// Get number of file formats
	u32 num_file_formats;
	archive->ReadU32(&num_file_formats);

	// Get file format index
	const u32 format_index_size = 12 * num_file_formats;
	std::vector<u8> format_index(format_index_size);
	archive->Read(format_index.data(), format_index.size());
	size_t format_index_ptr = 0;

	// Get index size
	u32 index_size = 0;
	for (u32 i = 0; i < num_file_formats; i++)
	{
		index_size += *(u32*)&format_index[12 * i + 4] * 17;
	}

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Get file information
	std::vector<SFileInfo> file_infos;
	std::vector<SFileInfo> mask_file_infos;
	std::vector<SFileInfo> not_mask_file_infos;

	size_t index_ptr = 0;
	for (u32 i = 0; i < num_file_formats; i++)
	{
		// Get filetype extension
		TCHAR file_extension[8];
		memcpy(file_extension, &format_index[format_index_ptr], 4);
		file_extension[4] = '\0';
		::CharLower(file_extension);

		// Get file information
		const u32 num_files = *(u32*)&format_index[format_index_ptr + 4];

		for (u32 j = 0; j < num_files; j++)
		{
			// Get file name
			char file_title[16];
			memcpy(file_title, &index[index_ptr], 9);
			file_title[9] = '\0';

			TCHAR file_name[32];
			_stprintf(file_name, _T("%s.%s"), file_title, file_extension);

			// Add information to the list
			SFileInfo file_info;
			file_info.name = file_name;
			file_info.size_cmp = *(u32*)&index[index_ptr + 9];
			file_info.size_org = file_info.size_cmp;
			file_info.start = *(u32*)&index[index_ptr + 13];
			file_info.end = file_info.start + file_info.size_cmp;

			if (lstrcmp(file_extension, _T("msk")) == 0)
			{
				// Masked image
				mask_file_infos.push_back(file_info);
			}
			else
			{
				file_infos.push_back(file_info);
			}

			index_ptr += 17;
		}

		format_index_ptr += 12;
	}

	// Sort by filename
	std::sort(file_infos.begin(), file_infos.end(), CArcFile::CompareForFileInfo);

	// Get file information from the mask image
	for (auto& mask_file_info : mask_file_infos)
	{
		// Get the name of the file to be created
		TCHAR target_name[_MAX_FNAME];
		lstrcpy(target_name, mask_file_info.name);
		PathRenameExtension(target_name, _T(".wip"));

		// Getting file information to be created
		SFileInfo* target_file_info = archive->SearchForFileInfo(file_infos, target_name);
		if (target_file_info != nullptr)
		{
			// Definitely the mask image
			target_file_info->starts.push_back(mask_file_info.start);
			target_file_info->sizes_cmp.push_back(mask_file_info.size_cmp);
			target_file_info->sizes_org.push_back(mask_file_info.size_org);

			// Progress update
			archive->GetProg()->UpdatePercent(mask_file_info.size_cmp);
		}
		else
		{
			// Is not a mask image
			not_mask_file_infos.push_back(mask_file_info);
		}
	}

	// Add to listview
	for (auto& file_info : file_infos)
	{
		archive->AddFileInfo(file_info);
	}

	for (auto& not_mask_file_info : not_mask_file_infos)
	{
		archive->AddFileInfo(not_mask_file_info);
	}

	return true;
}

/// Decoding
bool CWill::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->format != _T("WIP") && file_info->format != _T("MSK"))
		return false;

	// Read data
	std::vector<u8> src(file_info->size_cmp);
	archive->Read(src.data(), src.size());
	size_t src_ptr = 0;

	// Get number of files
	const u16 num_files = *(u16*)&src[4];
	src_ptr += 8;

	// Get width and height
	std::vector<s32> widths;
	std::vector<s32> heights;
	std::vector<u32> src_sizes;

	for (size_t i = 0; i < num_files; i++)
	{
		widths.push_back(*(s32*)&src[src_ptr + 0]);
		heights.push_back(*(s32*)&src[src_ptr + 4]);
		src_sizes.push_back(*(u32*)&src[src_ptr + 20]);

		src_ptr += 24;
	}

	// Is image mask there or not.
	bool mask_exists = !file_info->starts.empty();

	// Get image mask
	std::vector<u8>  mask_src;
	size_t           mask_src_ptr = 0;
	u16              mask_bpp = 0;
	std::vector<s32> mask_widths;
	std::vector<s32> mask_heights;
	std::vector<u32> mask_src_sizes;

	if (mask_exists)
	{
		// Image mask exists
		mask_src_ptr = 0;
		mask_src.resize(file_info->sizes_cmp[0]);

		// Read image mask
		archive->SeekHed(file_info->starts[0]);
		archive->Read(mask_src.data(), mask_src.size());

		// Get number of files and colors
		const u16 num_mask_files = *(u16*)&mask_src[4];
		mask_bpp = *(u16*)&mask_src[6];
		mask_src_ptr += 8;

		// Get width and height
		for (size_t i = 0; i < num_mask_files; i++)
		{
			mask_widths.push_back(*(s32*)&mask_src[mask_src_ptr + 0]);
			mask_heights.push_back(*(s32*)&mask_src[mask_src_ptr + 4]);
			mask_src_sizes.push_back(*(u32*)&mask_src[mask_src_ptr + 20]);

			mask_src_ptr += 24;
		}

		// Check to see if they have the same number of files
		mask_exists = num_files == num_mask_files;
	}

	// Output

	for (size_t i = 0; i < num_files; i++)
	{
		// マスク画像の付加で変更されるため再取得
		u16 bpp = *(u16*)&src[6];

		// Ensure the output buffer exists
		u32 dst_size = widths[i] * heights[i] * (bpp >> 3);
		std::vector<u8> dst(dst_size);
		u8* dst_ptr = dst.data();

		// Get pallet
		u8* pallet = nullptr;

		if (bpp == 8)
		{
			pallet = &src[src_ptr];
			src_ptr += 1024;
		}

		// LZSS Decompression
		DecompLZSS(dst.data(), dst.size(), &src[src_ptr], src_sizes[i]);
		src_ptr += src_sizes[i];

		// マスク画像を付加して32bit化
		std::vector<u8> dst_32bit;

		if (mask_exists)
		{
			// Image mask exists
			const u32 mask_dst_size = mask_widths[i] * mask_heights[i] * (mask_bpp >> 3);
			std::vector<u8> mask_dst(mask_dst_size);

			// Get pallet
			u8* mask_pallet = nullptr;

			if (mask_bpp == 8)
			{
				mask_pallet = &mask_src[mask_src_ptr];
				mask_src_ptr += 1024;
			}

			// LZSS Decompression
			DecompLZSS(mask_dst.data(), mask_dst.size(), &mask_src[mask_src_ptr], mask_src_sizes[i]);
			mask_src_ptr += mask_src_sizes[i];

			// Add mask to image
			const u32 dst_32bit_size = widths[i] * heights[i] * 4;
			dst_32bit.resize(dst_32bit_size);
			if (AppendMask(dst_32bit.data(), dst_32bit.size(), dst.data(), dst.size(), mask_dst.data(), mask_dst.size()))
			{
				// Success in adding the mask to the image
				bpp = 32;
				dst_size = dst_32bit_size;
				dst_ptr = dst_32bit.data();
			}
		}

		// Get file name
		TCHAR file_extension[256];

		if (num_files == 1)
		{
			// One file
			lstrcpy(file_extension, _T(""));
		}
		else
		{
			// Two or more files
			_stprintf(file_extension, _T("_%03zu.bmp"), i);
		}

		// Request progress bar progress
		const bool progress = i == 0;

		// Output
		CImage image;
		image.Init(archive, widths[i], heights[i], bpp, pallet, 1024, file_extension);
		image.WriteCompoBGRAReverse(dst_ptr, dst_size, progress);
		image.Close();
	}

	return true;
}

/// LZSS Decompression
///
/// Parameters:
/// @param dst      Destination
/// @param dst_size Destination Size
/// @param src      Compressed data
/// @param src_size Compressed data size
///
void CWill::DecompLZSS(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	size_t src_ptr = 0;
	size_t dst_ptr = 0;

	// Slide dictionary
	constexpr size_t dictionary_size = 4096;
	std::vector<u8> dictionary(dictionary_size);
	size_t dictionary_ptr = 1;

	while (src_ptr < src_size && dst_ptr < dst_size)
	{
		u8 flags = src[src_ptr++];

		for (size_t i = 0; i < 8 && src_ptr < src_size && dst_ptr < dst_size; i++)
		{
			if (flags & 1)
			{
				// Uncompressed data
				dst[dst_ptr++] = dictionary[dictionary_ptr++] = src[src_ptr++];
				dictionary_ptr &= dictionary_size - 1;
			}
			else
			{
				// Compressed data
				const u8 low = src[src_ptr++];
				const u8 high = src[src_ptr++];

				size_t back = ((low << 8) | high) >> 4;
				if (back == 0)
				{
					// Completed decompressing
					return;
				}

				// Get length from dictionary
				size_t length = (high & 0x0F) + 2;
				if (dst_ptr + length > dst_size)
				{
					// Exceeds the output buffer
					length = dst_size - dst_ptr;
				}

				// Enter data dictionary
				for (size_t j = 0; j < length; j++)
				{
					dst[dst_ptr++] = dictionary[dictionary_ptr++] = dictionary[back++];

					dictionary_ptr &= dictionary_size - 1;
					back &= dictionary_size - 1;
				}
			}

			flags >>= 1;
		}
	}
}

/// マスク画像を付加して32bit化する
///
/// Parameters:
/// @param dst       Destination
/// @param dst_size  Destination Size
/// @param src       24-bit data
/// @param src_size  24-bit data size
/// @param mask      8-bit data (mask)
/// @param mask_size 8-bit data size
///
bool CWill::AppendMask(u8* dst, size_t dst_size, const u8* src, size_t src_size, const u8* mask, size_t mask_size)
{
	// Make files
	memcpy(dst, src, src_size);
	memcpy(&dst[src_size], mask, mask_size);

	return true;
}
