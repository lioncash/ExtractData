#include "StdAfx.h"
#include "Extract/Himauri.h"

#include "ArcFile.h"
#include "Image.h"
#include "Image/Tga.h"
#include "Utils/BitUtils.h"

#include <locale>

/// Mount
///
/// @param archive Archive
///
bool CHimauri::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".hxp"))
		return false;

	if (MountHim4(archive))
		return true;

	if (MountHim5(archive))
		return true;

	return false;
}

/// Mount Him4
///
/// @param archive Archive
///
bool CHimauri::MountHim4(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "Him4", 4) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->SeekHed(4);
	archive->ReadU32(&num_files);

	// Get index size from file count
	const u32 index_size = num_files * 4;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());
	TCHAR archive_name[_MAX_FNAME];

	lstrcpy(archive_name, archive->GetArcName());
	PathRemoveExtension(archive_name);

	size_t index_ptr = 0;
	for (u32 i = 0; i < num_files; i++)
	{
		TCHAR file_name[_MAX_FNAME];
		_stprintf(file_name, _T("%s_%06u"), archive_name, i);

		// Add to list view
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = *(u32*)&index[index_ptr + 0];
		file_info.end = ((i + 1) < num_files) ? *(u32*)&index[index_ptr + 4] : archive->GetArcSize();
		file_info.sizeCmp = file_info.end - file_info.start;
		file_info.sizeOrg = file_info.sizeCmp;

		if (file_info.sizeCmp != 10)
		{
			// Is not a dummy file
			archive->AddFileInfo(file_info);
		}

		index_ptr += 4;
	}

	return true;
}

/// Mount Him5
///
/// @param archive Archive
///
bool CHimauri::MountHim5(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "Him5", 4) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->SeekHed(4);
	archive->ReadU32(&num_files);

	// Get index size from file count
	const u32 index_size = num_files * 8;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());
	size_t index_ptr = 0;

	// Gets index 2's size from the first index
	const u32 index2_size = *(u32*)&index[num_files * 8 - 4] - *(u32*)&index[4] + *(u32*)&index[num_files * 8 - 8];

	// Get index 2
	std::vector<u8> index2(index2_size);
	archive->Read(index2.data(), index2.size());
	size_t index2_ptr = 0;

	// Remove dummy
	for (u32 i = num_files; i > 0; i--)
	{
		if (*(u32*)&index[index_ptr + 0] == 0)
		{
			num_files--;
		}
	}

	// Gets difference increment
	const bool diff_compose = archive->GetArcName() == _T("natsucha.hxp");

	// Gets file info
	std::vector<SFileInfo> file_infos;
	std::vector<SFileInfo> diff_file_infos;

	for (u32 i = 0; i < num_files; i++)
	{
		while (true)
		{
			// Gets the length of a segment
			const u32 segment_length = index2[index2_ptr + 0];

			// Get the file name from index 2
			const u32 file_name_length = segment_length - 5;
			char file_name[_MAX_FNAME];
			memcpy(file_name, &index2[index2_ptr + 5], file_name_length);
			SFileInfo file_info;

			file_info.name = file_name;
			file_info.start = BitUtils::Swap32(*(u32*)&index2[index2_ptr + 1]);

			// Get the exit address

			if (index2[index2_ptr + segment_length] == 0)
			{
				if ((i + 1) == num_files)
				{
					file_info.end = archive->GetArcSize();
				}
				else
				{
					file_info.end = BitUtils::Swap32(*(u32*)&index2[index2_ptr + segment_length + 2]);
				}
			}
			else
			{
				file_info.end = BitUtils::Swap32(*(u32*)&index2[index2_ptr + segment_length + 1]);
			}

			file_info.sizeCmp = file_info.end - file_info.start;
			file_info.sizeOrg = file_info.sizeCmp;

			if (file_info.sizeCmp != 10)
			{
				// Is not a dummy file
				if (diff_compose)
				{
					// Difference composition is enabled
					const char* diff_mark = strrchr(file_name, _T('_'));

					if (diff_mark != nullptr && strlen(diff_mark) >= 2 && std::isdigit(diff_mark[1], std::locale::classic()))
					{
						// Difference file
						const int pos = file_info.name.ReverseFind(_T('_'));
						file_info.name.Delete(pos + 1, file_info.name.GetLength());
						diff_file_infos.push_back(file_info);
					}
					else
					{
						file_infos.push_back(file_info);
					}
				}
				else
				{
					archive->AddFileInfo(file_info);
				}
			}

			// 1セグメントの最後が0なら1ファイル終了と判断

			if (index2[index2_ptr + segment_length] == 0)
			{
				index2_ptr += segment_length + 1;
				break;
			}

			index2_ptr += segment_length;
		}
	}

	if (diff_compose)
	{
		// Difference composition is enabled
		for (auto& file_info : file_infos)
		{
			const int pos = file_info.name.ReverseFind(_T('_'));
			if (pos >= 0)
			{
				YCString base_file_name = file_info.name.Left(pos + 1);

				for (auto& diff_file_info : diff_file_infos)
				{
					if (base_file_name == diff_file_info.name)
					{
						// Within the difference range
						file_info.starts.push_back(diff_file_info.start);
						file_info.sizesCmp.push_back(diff_file_info.sizeCmp);
						file_info.sizesOrg.push_back(diff_file_info.sizeOrg);
					}
				}
			}

			archive->AddFileInfo(file_info);
		}
	}

	return true;
}

/// Decode
///
/// @param archive Archive
///
bool CHimauri::Decode(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".hxp"))
		return false;

	if (memcmp(archive->GetHeader(), "Him", 3) != 0)
		return false;

	// Get input size
	u32 src_size;
	archive->ReadU32(&src_size);

	// Get output size
	u32 dst_size;
	archive->ReadU32(&dst_size);

	// Ensure buffer
	std::vector<u8> dst(dst_size);

	if (src_size == 0)
	{
		// Not a compressed file
		archive->Read(dst.data(), dst.size());
	}
	else
	{
		// Compressed file
		std::vector<u8> src(src_size);
		archive->Read(src.data(), src.size());
		Decomp(dst.data(), dst.size(), src.data(), src.size());
	}

	YCString file_extension;
	if (memcmp(dst.data(), "OggS", 4) == 0)
	{
		// Ogg Vorbis
		file_extension = _T(".ogg");
	}
	else if (memcmp(dst.data(), "RIFF", 4) == 0)
	{
		// WAVE
		file_extension = _T(".wav");
	}
	else if (memcmp(dst.data(), "Himauri", 7) == 0)
	{
		// hst
		file_extension = _T(".txt");
	}
	else if (memcmp(dst.data(), "BM", 2) == 0)
	{
		// BMP
		CImage image;
		image.Init(archive, dst.data());
		image.Write(dst.size());

		return true;
	}
	else if (dst[0] == 0 && dst[1] <= 1 && dst[2] > 0 && memcmp(&dst[3], "\x00\x00\x00\x00\x00", 5) == 0)
	{
		// TGA
		const SFileInfo* file_info = archive->GetOpenFileInfo();

		if (file_info->starts.empty())
		{
			// Difference does not exist
			CTga tga;
			tga.Decode(archive, dst.data(), dst.size());
		}
		else
		{
			// There is a difference

			// Get TGA image-based header
			const auto* tga_header = reinterpret_cast<const CTga::TGAHeader*>(&dst[0]);

			// TGA Decompression
			const u32 dst2_size = ((tga_header->width * (tga_header->depth >> 3) + 3) & 0xFFFFFFFC) * tga_header->height;
			std::vector<u8> dst2(dst2_size);

			CTga tga;
			tga.Decomp(dst2.data(), dst2.size(), dst.data(), dst.size());

			// Outputs difference
			for (size_t i = 0; i < file_info->starts.size(); i++)
			{
				archive->SeekHed(file_info->starts[i]);

				// Get image input size difference
				u32 src_diff_size;
				archive->ReadU32(&src_diff_size);

				// Get image output size difference
				u32 dst_diff_size;
				archive->ReadU32(&dst_diff_size);

				// Ensure image difference buffer
				std::vector<u8> dst_diff(dst_diff_size);

				// Get image difference
				if (src_diff_size == 0)
				{
					// Uncompressed file
					archive->Read(dst_diff.data(), dst_diff.size());
				}
				else
				{
					// Compressed file
					std::vector<u8> src_diff(src_diff_size);
					archive->Read(src_diff.data(), src_diff.size());
					Decomp(dst_diff.data(), dst_diff.size(), src_diff.data(), src_diff.size());
				}

				// Get TGA image header difference
				const auto* tga_header_diff = reinterpret_cast<const CTga::TGAHeader*>(&dst_diff[0]);

				// TGA Decompression
				const u32 dst_diff2_size = ((tga_header_diff->width * (tga_header_diff->depth >> 3) + 3) & 0xFFFFFFFC) * tga_header_diff->height;
				std::vector<u8> dst_diff2(dst_diff2_size);
				tga.Decomp(dst_diff2.data(), dst_diff2.size(), dst_diff.data(), dst_diff.size());

				// Difference Composition
				const u32 dst_compose_size = dst2_size;
				std::vector<u8> dst_compose(dst_compose_size);
				Compose(dst_compose.data(), dst_compose.size(), dst2.data(), dst2.size(), dst_diff2.data(), dst_diff2.size());

				// End of filename changes
				TCHAR last_name[_MAX_FNAME];
				_stprintf(last_name, _T("_%03zu.bmp"), i);

				// Request progress bar progress
				const bool progress = i == 0;

				// Output
				CImage image;
				image.Init(archive, tga_header->width, tga_header->height, tga_header->depth, nullptr, 0, last_name);
				image.Write(dst_compose.data(), dst_compose.size(), progress);
				image.Close();
			}
		}

		return true;
	}
	else if (memcmp(dst.data(), "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16) == 0)
	{
		// Mask image
		CImage image;
		image.Init(archive, 800, 600, 8);
		image.WriteReverse(dst.data(), dst.size());

		return true;
	}
	else
	{
		// Other
		file_extension = _T(".txt");
	}

	archive->OpenFile(file_extension);
	archive->WriteFile(dst.data(), dst.size(), archive->GetOpenFileInfo()->sizeCmp);

	return true;
}

/// Decompression
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Compressed data
/// @param src_size Compressed data size
///
void CHimauri::Decomp(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	size_t src_ptr = 0;
	size_t dst_ptr = 0;
	u32 code = 0;
	u32 back = 0;

	while (src_ptr < src_size && dst_ptr < dst_size)
	{
		if (code == 0)
		{
			code = src[src_ptr++];

			if (code < 0x20)
			{
				back = 0;

				if (code < 0x1D)
				{
					code++;
				}
				else if (code == 0x1D)
				{
					code = src[src_ptr++] + 0x1E;
				}
				else if (code == 0x1E)
				{
					code = src[src_ptr++];
					code <<= 8;
					code |= src[src_ptr++];
					code += 0x11E;
				}
				else if (code == 0x1F)
				{
					code = src[src_ptr++];
					code <<= 8;
					code |= src[src_ptr++];
					code <<= 8;
					code |= src[src_ptr++];
					code <<= 8;
					code |= src[src_ptr++];
				}
			}
			else
			{
				if (code >= 0x80)
				{
					back = ((code & 0x1F) << 8) | src[src_ptr++];
					code = (code >> 5) & 3;
				}
				else
				{
					const u32 code2 = code & 0x60;

					if (code2 == 0x20)
					{
						back = (code >> 2) & 7;
						code &= 3;
					}
					else
					{
						code &= 0x1F;

						if (code2 == 0x40)
						{
							back = src[src_ptr++];
							code += 4;
						}
						else
						{
							back = (code << 8) | src[src_ptr++];
							code = src[src_ptr++];

							if (code == 0xFE)
							{
								code = src[src_ptr++];
								code <<= 8;
								code |= src[src_ptr++];
								code += 0x102;
							}
							else if (code == 0xFF)
							{
								code = src[src_ptr++];
								code <<= 8;
								code |= src[src_ptr++];
								code <<= 8;
								code |= src[src_ptr++];
								code <<= 8;
								code |= src[src_ptr++];
							}
							else
							{
								code += 4;
							}
						}
					}
				}

				back++;
				code += 3;
			}
		}

		// Get output length
		u32 length = code;
		if (dst_ptr + length > dst_size)
		{
			length = dst_size - dst_ptr;
		}

		code -= length;

		// Output
		if (back > 0)
		{
			// Output previous data
			for (size_t i = 0; i < length; i++)
			{
				dst[dst_ptr + i] = dst[dst_ptr - back + i];
			}

			dst_ptr += length;
		}
		else
		{
			// Output input data
			memcpy(&dst[dst_ptr], &src[src_ptr], length);

			src_ptr += length;
			dst_ptr += length;
		}
	}
}

/// Difference Composition
///
/// @param dst       Destination
/// @param dst_size  Destination Size
/// @param base      Image base
/// @param base_size Image base size
/// @param diff      Image difference
/// @param diff_size Image difference size
///
bool CHimauri::Compose(u8* dst, size_t dst_size, const u8* base, size_t base_size, const u8* diff, size_t diff_size)
{
	// Synthesize base image and difference image
	memcpy(dst, base, dst_size);

	for (size_t i = 0; i < dst_size; i += 4)
	{
		// 32bit -> 24bit

		if (diff[i + 3] > 0)
		{
			for (size_t j = 0; j < 3; j++)
			{
				dst[i + j] = (diff[i + j] - base[i + j]) * diff[i + 3] / 255 + base[i + j];
			}

			dst[i + 3] = 0xFF;
		}
	}

	return true;
}
