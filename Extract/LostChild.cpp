#include "StdAfx.h"
#include "../Image.h"
#include "LostChild.h"

/// Mounting
///
/// @param archive Archive
///
bool CLostChild::Mount(CArcFile* archive)
{
	if (memcmp(archive->GetHed(), "EPK ", 4) != 0)
		return false;

	archive->SeekHed(4);

	// Get index size
	u32 index_size;
	archive->ReadU32(&index_size);
	index_size -= 32;
	archive->SeekCur(16);

	// Get file count
	u32 num_files;
	archive->ReadU32(&num_files);
	archive->SeekCur(4);

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Get the filename index
	const u8* file_name_index = &index[*reinterpret_cast<const u32*>(&index[8])] - 32;

	// Split archive files
	if (archive->GetArcName() == _T("data.epk"))
	{
		YCString archive_path = archive->GetArcPath();

		for (u32 i = 1; i <= 3; i++)
		{
			YCString archive_extension;
			archive_extension.Format(_T(".e%02u"), i);
			archive_path.RenameExtension(archive_extension);
			archive->Open(archive_path);
			archive->GetProg()->ReplaceAllFileSize(archive->GetArcSize());
		}

		archive->SetFirstArc();
	}

	// Get file information
	size_t index_ptr = 0;
	size_t file_name_index_ptr = 0;

	for (u32 i = 0; i < num_files; i++)
	{
		// Get filename
		char  file_name[_MAX_FNAME];
		const u32 length = *reinterpret_cast<const u32*>(&file_name_index[file_name_index_ptr]);

		for (size_t j = 0; j < length; j++)
		{
			file_name[j] = file_name_index[file_name_index_ptr + 4 + j] ^ 0xFF;
		}

		file_name[length] = file_name_index[file_name_index_ptr + 4 + length];
		file_name_index_ptr += 4 + length + 1;

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = *reinterpret_cast<const u64*>(&index[index_ptr + 16]);
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&index[index_ptr + 24]);
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.title = _T("LOST CHILD");

		index_ptr += 40;

		if (file_info.start > 3900000000)
		{
			file_info.start -= 3900000000;

			if (archive->GetArcsID() == 2)
			{
				archive->SetNextArc();
			}
		}
		else if (file_info.start > 2600000000)
		{
			file_info.start -= 2600000000;

			if (archive->GetArcsID() == 1)
			{
				archive->SetNextArc();
			}
		}
		else if (file_info.start > 1300000000)
		{
			file_info.start -= 1300000000;

			if (archive->GetArcsID() == 0)
			{
				archive->SetNextArc();
			}
		}

		// File size adjustment exceeds 1.2GB when adding
		file_info.end = file_info.start + file_info.sizeCmp;
		if (file_info.end > 1300000000)
		{
			file_info.end -= 1300000000;
		}

		archive->AddFileInfo(file_info);
	}

	archive->SetFirstArc();

	return true;
}

/// Decoding
///
/// @param archive Archive
///
bool CLostChild::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->title != _T("LOST CHILD"))
		return false;

	if (DecodeESUR(archive))
		return true;

	if (DecodeLAD(archive))
		return true;

	return Extract(archive);
}

/// ESUR Decoding
///
/// @param archive Archive
///
bool CLostChild::DecodeESUR(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->format != _T("SUR"))
		return false;

	// Read
	const size_t src_size = file_info->sizeCmp;
	std::vector<u8> src(src_size);
	const size_t read_size = archive->Read(src.data(), src.size());

	if (read_size < src_size)
	{
		archive->SetNextArc();
		archive->SeekHed();

		archive->Read(&src[read_size], src_size - read_size);
	}

	// Get header information
	const s32 width = *reinterpret_cast<const s32*>(&src[8]);
	const s32 height = *reinterpret_cast<const s32*>(&src[12]);
	const u32 dst_size = *reinterpret_cast<const u32*>(&src[4]) - 32;
	const u16 bpp = 32;

	// Get a buffer for LZSS decompression
	std::vector<u8> dst(dst_size);

	// LZSS Decompression
	DecompLZSS(dst.data(), dst.size(), &src[32], src_size - 32, 4096, 4078, 3);

	// Output
	CImage image;
	image.Init(archive, width, height, bpp);
	image.WriteReverse(dst.data(), dst.size());
	image.Close();

	return true;
}

/// LAD Decoding
///
/// @param archive Archive
///
bool CLostChild::DecodeLAD(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->format != _T("LAD"))
		return false;

	// Reading
	const size_t src_size = file_info->sizeCmp;
	std::vector<u8> src(src_size);
	const size_t read_size = archive->Read(src.data(), src.size());

	if (read_size < src_size)
	{
		archive->SetNextArc();
		archive->SeekHed();

		archive->Read(&src[read_size], src_size - read_size);
	}

	// Get header info
	const s32 width = *reinterpret_cast<const s32*>(&src[8]);
	const s32 height = *reinterpret_cast<const s32*>(&src[12]);
	const u32 dst_size = *reinterpret_cast<const u32*>(&src[28]);
	const u16 bpp = 8;

	// Get a buffer for LZSS decoding
	std::vector<u8> dst(dst_size);

	// LZSS Decompression
	DecompLZSS(dst.data(), dst.size(), &src[32], src_size - 32, 4096, 4078, 3);

	archive->OpenFile();
	archive->WriteFile(dst.data(), dst.size(), src.size());
	archive->CloseFile();

	// Output
//	CImage image;
//	image.Init(archive, width, height, bpp);
//	image.WriteReverse(dst.data(), dst.size());
//	image.Close();

	return true;
}

/// LZSS Decompression
///
/// @param dst             Destination
/// @param dst_size        Destination size
/// @param src             Input data
/// @param src_size        Input data size
/// @param dictionary_size Dictionary size
/// @param dictionary_ptr  Dictionary reference position
/// @param length_offset   Length offset
///
bool CLostChild::DecompLZSS(u8* dst, size_t dst_size, const u8* src, size_t src_size, size_t dictionary_size, size_t dictionary_ptr, size_t length_offset)
{
	// Allocate buffer
	std::vector<u8> dictionary(dictionary_size);

	// Decoding
	size_t src_ptr = 0;
	size_t dst_ptr = 0;
	u8  flags = 0;
	u32 bit_count = 0;

	while (src_ptr < src_size && dst_ptr < dst_size)
	{
		if (bit_count == 0)
		{
			// 8-bit reading
			flags = src[src_ptr++];
			bit_count = 8;
		}

		if (flags & 1)
		{
			// Uncompressed data

			dst[dst_ptr] = dictionary[dictionary_ptr] = src[src_ptr];

			dst_ptr++;
			src_ptr++;
			dictionary_ptr++;

			dictionary_ptr &= dictionary_size - 1;
		}
		else
		{
			// Compressed data

			const u8 low = src[src_ptr++];
			const u8 high = src[src_ptr++];

			u32 back = (low << 4) | (high >> 4);
			u32 length = (high & 0x0F) + length_offset;

			if (dst_ptr + length > dst_size)
			{
				// Exceeds the output buffer

				length = dst_size - dst_ptr;
			}

			for (size_t j = 0; j < length; j++)
			{
				dst[dst_ptr] = dictionary[dictionary_ptr] = dictionary[back];

				dst_ptr++;
				dictionary_ptr++;
				back++;

				dictionary_ptr &= dictionary_size - 1;
				back &= dictionary_size - 1;
			}
		}

		flags >>= 1;
		bit_count--;
	}

	return true;
}

/// Extraction
///
/// @param archive Archive
///
bool CLostChild::Extract(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->title != _T("LOST CHILD"))
		return false;

	// Ensure buffer exists
	size_t buffer_size = archive->GetBufSize();
	std::vector<u8> buffer(buffer_size);

	// Generate output files
	archive->OpenFile();

	for (size_t write_size = 0; write_size != file_info->sizeCmp; write_size += buffer_size)
	{
		// Adjust buffer size
		archive->SetBufSize(&buffer_size, write_size);

		// Read
		const size_t read_size = archive->Read(buffer.data(), buffer_size);
		archive->WriteFile(buffer.data(), read_size);

		if (read_size < buffer_size)
		{
			archive->SetNextArc();

			archive->SeekHed();
			archive->Read(buffer.data(), buffer_size - read_size);
			archive->WriteFile(buffer.data(), buffer_size - read_size);
		}
	}

	archive->CloseFile();

	return true;
}
