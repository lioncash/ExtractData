#include "StdAfx.h"
#include "Extract/Nscr.h"

#include "../bzip2/bzlib.h"
#include "ArcFile.h"
#include "Common.h"
#include "Image.h"
#include "Utils/BitUtils.h"

/// Mounting
///
/// @param archive Archive
///
bool CNscr::Mount(CArcFile* archive)
{
	if (MountNsa(archive))
		return true;

	if (MountSar(archive))
		return true;

	if (MountScr(archive))
		return true;

	return false;
}

/// nsa mounting
///
/// @param archive Archive
///
bool CNscr::MountNsa(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".nsa"))
		return false;

	// Get file count
	u32 num_files = 0;

	if (memcmp(archive->GetHeader(), "\0\0", 2) == 0)
	{
		// Each 4 bytes
		archive->Read(&num_files, 4);
	}
	else
	{
		archive->Read(&num_files, 2);
		num_files <<= 16;
	}

	num_files = BitUtils::Swap32(num_files);

	// Get offset
	u32 offset;
	archive->ReadU32(&offset);
	offset = BitUtils::Swap32(offset);

	if (memcmp(archive->GetHeader(), "\0\0", 2) == 0)
	{
		offset += 2;
	}

	// Get index size
	u32 index_size = offset;
	index_size -= (memcmp(archive->GetHeader(), "\0\0", 2) == 0) ? 8 : 6;

	// Get the index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	size_t index_ptr = 0;
	for (u32 i = 0; i < num_files; i++)
	{
		// Get file name
		TCHAR file_name[256];
		lstrcpy(file_name, (LPCTSTR)&index[index_ptr]);
		index_ptr += lstrlen(file_name) + 1;

		const u8 type = index[index_ptr];

		// Add to list view
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = BitUtils::Swap32(*(u32*)&index[index_ptr + 1]) + offset;
		file_info.size_cmp = BitUtils::Swap32(*(u32*)&index[index_ptr + 5]);
		file_info.size_org = BitUtils::Swap32(*(u32*)&index[index_ptr + 9]);
		file_info.end = file_info.start + file_info.size_cmp;

		switch (type)
		{
		case 1:
			file_info.format = _T("SPB");
			break;

		case 2:
			file_info.format = _T("LZSS");
			break;
		}

		archive->AddFileInfo(file_info);

		index_ptr += 13;
	}

	return true;
}

/// sar mounting
///
/// @param archive Archive
///
bool CNscr::MountSar(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".sar"))
		return false;

	// Get file count
	u16 num_files;
	archive->ReadU16(&num_files);
	num_files = BitUtils::Swap16(num_files);

	// Get offset
	u32 offset;
	archive->ReadU32(&offset);
	offset = BitUtils::Swap32(offset);

	// Get index size
	const u32 index_size = offset - 6;

	// Get the index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	size_t index_ptr = 0;
	for (u32 i = 0; i < num_files; i++)
	{
		// Get file name
		TCHAR file_name[256];
		lstrcpy(file_name, (LPCTSTR)&index[index_ptr]);
		index_ptr += lstrlen(file_name) + 1;

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = BitUtils::Swap32(*(u32*)&index[index_ptr + 0]) + offset;
		file_info.size_org = BitUtils::Swap32(*(u32*)&index[index_ptr + 4]);
		file_info.size_cmp = file_info.size_org;
		file_info.end = file_info.start + file_info.size_cmp;

		archive->AddFileInfo(file_info);

		index_ptr += 8;
	}

	return true;
}

/// Script file mounting
///
/// @param archive Archive
///
bool CNscr::MountScr(CArcFile* archive)
{
	if (archive->GetArcName() != _T("nscript.dat"))
		return false;

	return archive->Mount();
}

/// Decoding
///
/// @param archive Archive
///
bool CNscr::Decode(CArcFile* archive)
{
	if (DecodeScr(archive))
		return true;

	if (DecodeSPB(archive))
		return true;

	if (DecodeLZSS(archive))
		return true;

	if (DecodeNBZ(archive))
		return true;

	return false;
}

/// Script file decoding
///
/// @param archive Archive
///
bool CNscr::DecodeScr(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->name != _T("nscript.dat"))
		return false;

	// Ensure buffer exists
	size_t buffer_size = archive->GetBufSize();
	std::vector<u8> buffer(buffer_size);
	std::vector<u8> buffer2(buffer_size * 2);

	// Generate output files
	archive->OpenFile(_T(".txt"));

	for (size_t write_size = 0; write_size != file_info->size_org; write_size += buffer_size)
	{
		// Buffer adjustment
		archive->SetBufSize(&buffer_size, write_size);

		// Reading
		archive->Read(buffer.data(), buffer_size);

		size_t buffer2_size = 0;

		for (size_t i = 0; i < buffer_size; i++)
		{
			// Decoding
			buffer2[buffer2_size] = buffer[i] ^ 0x84;

			// Change to CR + LF line endings
			if (buffer2[buffer2_size] == '\n')
			{
				buffer2[buffer2_size++] = '\r';
				buffer2[buffer2_size] = '\n';
			}

			buffer2_size++;
		}

		archive->WriteFile(buffer2.data(), buffer2_size);
	}

	return true;
}

/// NBZ Decoding
///
/// @param archive Archive
///
bool CNscr::DecodeNBZ(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->format != _T("NBZ"))
		return false;

	// Get file size
	u32 dst_size;
	archive->ReadU32(&dst_size);
	dst_size = BitUtils::Swap32(dst_size);

	const u32 src_size = file_info->size_cmp - 4;

	// Ensure buffer exists
	std::vector<u8> src(src_size);
	std::vector<u8> dst(dst_size);

	// NBZ decompression
	archive->Read(src.data(), src.size());
	BZ2_bzBuffToBuffDecompress(reinterpret_cast<char*>(dst.data()), &dst_size,
	                           reinterpret_cast<char*>(src.data()), src_size, 0, 0);

	// Obtain file extension
	YCString file_extension;
	GetFileExt(file_extension, dst.data());

	// Output
	archive->OpenFile(file_extension);
	archive->WriteFile(dst.data(), dst.size());

	return true;
}

/// Getting file extensions
///
/// @param dst    Destination
/// @param buffer Buffer
///
void CNscr::GetFileExt(YCString& dst, const u8* buffer)
{
	if (memcmp(buffer, "BM", 2) == 0)
	{
		dst = _T(".bmp");
	}
	else if (memcmp(buffer, "RIFF", 4) == 0 && memcmp(&buffer[8], "WAVEfmt ", 8) == 0)
	{
		dst = _T(".wav");
	}
	else if (memcmp(buffer, "MThd", 4) == 0)
	{
		dst = _T(".mid");
	}
}

/// Get a stream of bits
///
/// @param src              Input data
/// @param num_bits_to_read Number of bits to read
/// @param num_bytes_read   Number of bytes read
///
u32 CNscr::GetBit(const u8* src, size_t num_bits_to_read, size_t* num_bytes_read)
{
	u32 result = 0;
	size_t src_ptr = 0;

	for (size_t i = 0; i < num_bits_to_read; i++)
	{
		if (m_mask_for_get_bit == 0)
		{
			m_src_for_get_bit = src[src_ptr++];
			m_mask_for_get_bit = 0x80;
		}

		result <<= 1;

		if (m_src_for_get_bit & m_mask_for_get_bit)
		{
			result++;
		}

		m_mask_for_get_bit >>= 1;
	}

	*num_bytes_read = src_ptr;

	return result;
}

/// SPB Decoding
///
/// @param archive Archive
///
bool CNscr::DecodeSPB(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->format != _T("SPB"))
		return false;

	// Get the width
	u16 width;
	archive->ReadU16(&width);
	width = BitUtils::Swap16(width);

	// Get the height
	u16 height;
	archive->ReadU16(&height);
	height = BitUtils::Swap16(height);

	// Parameters for the image
	const u32 color_size = width * height;
	const u32 line = width * 3;
	const u32 pitch = (line + 3) & 0xFFFFFFFC;

	// Ensure buffers exist
	const size_t src_size = file_info->size_cmp - 4;
	const size_t dst_size = file_info->size_org - 54;
	const size_t work_size = (color_size + 4) * 3;

	std::vector<u8> src(src_size);
	std::vector<u8> dst(dst_size);
	std::vector<u8> work_buffer(work_size);

	// Read the SPB file
	archive->Read(src.data(), src.size());

	// SPB decompression
	size_t src_ptr = 0;
	size_t work_ptr = 0;
	std::array<size_t, 3> saved_rgb_offsets;

	m_mask_for_get_bit = 0;
	m_src_for_get_bit = 0;

	for (auto& saved_rgb_offset : saved_rgb_offsets)
	{
		saved_rgb_offset = work_ptr;

		size_t num_bytes_read;
		u32 data = GetBit(&src[src_ptr], 8, &num_bytes_read);
		src_ptr += num_bytes_read;

		work_buffer[work_ptr++] = static_cast<u8>(data);

		// Decompression

		for (size_t count = 1; count < color_size; count += sizeof(u32))
		{
			const u32 flags = GetBit(&src[src_ptr], 3, &num_bytes_read);
			u32 flags2;

			src_ptr += num_bytes_read;

			switch (flags)
			{
			case 0:
				work_buffer[work_ptr + 0] = static_cast<u8>(data);
				work_buffer[work_ptr + 1] = static_cast<u8>(data);
				work_buffer[work_ptr + 2] = static_cast<u8>(data);
				work_buffer[work_ptr + 3] = static_cast<u8>(data);

				work_ptr += 4;
				continue;

			case 7:
				flags2 = GetBit(&src[src_ptr], 1, &num_bytes_read) + 1;
				src_ptr += num_bytes_read;
				break;

			default:
				flags2 = flags + 2;
				break;
			}

			for (size_t j = 0; j < 4; j++)
			{
				if (flags2 == 8)
				{
					data = GetBit(&src[src_ptr], 8, &num_bytes_read);
				}
				else
				{
					const u32 work = GetBit(&src[src_ptr], flags2, &num_bytes_read);

					if (work & 1)
					{
						data += (work >> 1) + 1;
					}
					else
					{
						data -= (work >> 1);
					}
				}

				src_ptr += num_bytes_read;
				work_buffer[work_ptr++] = static_cast<u8>(data);
			}
		}
	}

	// RGB synthesis
	u8* dst_ptr = &dst[pitch * (height - 1)];
	std::array<const u8*, 3> rgb_channel_ptrs{{
		&work_buffer[saved_rgb_offsets[0]],
		&work_buffer[saved_rgb_offsets[1]],
		&work_buffer[saved_rgb_offsets[2]]
	}};

	for (size_t j = 0; j < height; j++)
	{
		if (j & 1)
		{
			// Odd line
			for (size_t k = 0; k < width; k++)
			{
				dst_ptr[0] = *rgb_channel_ptrs[0]++;
				dst_ptr[1] = *rgb_channel_ptrs[1]++;
				dst_ptr[2] = *rgb_channel_ptrs[2]++;

				dst_ptr -= 3;
			}

			dst_ptr -= pitch - 3;
		}
		else
		{
			// Even line
			for (size_t k = 0; k < width; k++)
			{
				dst_ptr[0] = *rgb_channel_ptrs[0]++;
				dst_ptr[1] = *rgb_channel_ptrs[1]++;
				dst_ptr[2] = *rgb_channel_ptrs[2]++;

				dst_ptr += 3;
			}

			dst_ptr -= pitch + 3;
		}
	}

	// Output
	CImage image;
	image.Init(archive, width, height, 24);
	image.Write(dst.data(), dst.size());

	return true;
}

/// LZSS Decoding
///
/// @param archive Archive
///
bool CNscr::DecodeLZSS(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->format != _T("LZSS"))
		return false;

	// Ensure buffers exist
	const u32 src_size = file_info->size_cmp;
	const u32 dst_size = file_info->size_org;
	const u32 dic_size = 256;

	std::vector<u8> src(src_size);
	std::vector<u8> dst(dst_size);
	std::vector<u8> dictionary(dic_size);

	// Read
	archive->Read(src.data(), src.size());

	// LZSS decompression
	size_t src_ptr = 0;
	size_t dst_ptr = 0;
	size_t dic_ptr = 239;

	m_mask_for_get_bit = 0;
	m_src_for_get_bit = 0;

	while (dst_ptr < dst_size)
	{
		// Get the compression flag
		size_t num_bytes_read;
		const u32 flag = GetBit(&src[src_ptr], 1, &num_bytes_read);
		src_ptr += num_bytes_read;

		// Uncompressed
		if (flag & 1)
		{
			const u32 data = GetBit(&src[src_ptr], 8, &num_bytes_read);

			dst[dst_ptr++] = static_cast<u8>(data);
			dictionary[dic_ptr++] = static_cast<u8>(data);

			src_ptr += num_bytes_read;
			dic_ptr &= dic_size - 1;
		}
		else // Compressed
		{
			u32 back = GetBit(&src[src_ptr], 8, &num_bytes_read);
			src_ptr += num_bytes_read;

			u32 length = GetBit(&src[src_ptr], 4, &num_bytes_read) + 2;
			src_ptr += num_bytes_read;

			if (dst_ptr + length > dst_size)
			{
				// Larger than the output buffer
				length = dst_size - dst_ptr;
			}

			for (size_t i = 0; i < length; i++)
			{
				dst[dst_ptr++] = dictionary[dic_ptr++] = dictionary[back++];

				dic_ptr &= dic_size - 1;
				back &= dic_size - 1;
			}
		}
	}

	// Output
	CImage image;
	image.Init(archive, dst.data());
	image.Write(dst.size());

	return true;
}
