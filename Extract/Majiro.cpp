/*
   Copyright (c) 2005 Peter Jolly

   Permission is hereby granted, free of charge, to any person obtaining a copy 
   of this software and associated documentation files (the "Software"), to deal 
   in the Software without restriction, including without limitation the rights 
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
   copies of the Software, and to permit persons to whom the Software is 
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in 
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
   SOFTWARE.
*/

#include "StdAfx.h"
#include "Extract/Majiro.h"

#include "ArcFile.h"
#include "Image.h"

namespace
{
struct SRCHeader
{
	char identifier[8];
	s32  width;
	s32  height;
	u32  data_size;
};
} // Anonymous namespace

/// Mounting
///
/// @param archive Archive
///
bool CMajiro::Mount(CArcFile* archive)
{
	if (MountArc1(archive))
		return true;

	if (MountArc2(archive))
		return true;

	if (MountMJO(archive))
		return true;

	if (MountRC(archive))
		return true;

	return false;
}

/// Mounting V1 Archives
///
/// @param archive Archive
///
bool CMajiro::MountArc1(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".arc"))
		return false;

	if (memcmp(archive->GetHeader(), "MajiroArcV1.000", 15) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->SeekHed(16);
	archive->ReadU32(&num_files);

	// Get the index size of the file entry
	u32 file_entry_index_size;
	archive->ReadU32(&file_entry_index_size);
	file_entry_index_size -= 28;

	// Get index size
	u32 index_size;
	archive->ReadU32(&index_size);
	index_size -= 28;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Get the index filename
	const u8* file_name_index_ptr = &index[file_entry_index_size];

	// Get file information
	std::vector<SFileInfo> file_infos;
	std::vector<SFileInfo> mask_file_infos;
	std::vector<SFileInfo> not_mask_file_infos;

	for (u32 i = 0; i < num_files; i++)
	{
		SFileInfo file_info;

		// Get the filename from the filename index
		TCHAR file_name[256];
		lstrcpy(file_name, reinterpret_cast<LPCTSTR>(file_name_index_ptr));
		file_info.name = file_name;

		// Get the starting and ending addresses from the index
		file_info.start = *reinterpret_cast<u32*>(&index[i * 8 + 4]);
		file_info.end = *reinterpret_cast<u32*>(&index[i * 8 + 12]);

		// Get file sizes
		file_info.size_org = file_info.end - file_info.start;
		file_info.size_cmp = file_info.size_org;

		// Add file information to list
		if (lstrcmpi(PathFindExtension(file_info.name), _T(".rc8")) == 0)
		{
			if (file_info.name[file_info.name.GetLength() - lstrlen(_T("_.rc8"))] == _T('_'))
			{
				mask_file_infos.push_back(file_info);
			}
			else
			{
				not_mask_file_infos.push_back(file_info);
			}
		}
		else
		{
			file_infos.push_back(file_info);
		}

		file_name_index_ptr += lstrlen(file_name) + 1;
	}

	// Sort by filename
	std::sort(file_infos.begin(), file_infos.end(), CArcFile::CompareForFileInfo);

	// Get file information from the masked image
	for (auto& mask_file_info : mask_file_infos)
	{
		// Get the name of the file to be created
		TCHAR rct_name[_MAX_FNAME];
		lstrcpy(rct_name, mask_file_info.name);
		rct_name[lstrlen(rct_name) - lstrlen(_T("_.rc8"))] = _T('\0');
		PathRenameExtension(rct_name, _T(".rct"));

		// Get the file information to be created
		SFileInfo* target_file_info = archive->SearchForFileInfo(file_infos, rct_name);
		if (target_file_info != nullptr)
		{
			// Image is masked
			target_file_info->starts.push_back(mask_file_info.start);
			target_file_info->sizes_cmp.push_back(mask_file_info.size_cmp);
			target_file_info->sizes_org.push_back(mask_file_info.size_org);

			// Update progress
			archive->GetProg()->UpdatePercent(mask_file_info.size_cmp);
		}
		else
		{
			// Image is not masked
			not_mask_file_infos.push_back(mask_file_info);
		}
	}

	// Add to list view
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

/// Mounting V2 Archives
///
/// @param archive Archive
///
bool CMajiro::MountArc2(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".arc"))
		return false;

	if (memcmp(archive->GetHeader(), "MajiroArcV2.000", 15) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->SeekHed(16);
	archive->ReadU32(&num_files);

	// Get the index size of the file entry
	u32 file_entry_index_size;
	archive->ReadU32(&file_entry_index_size);
	file_entry_index_size -= 28;

	// Get index size
	u32 index_size;
	archive->ReadU32(&index_size);
	index_size -= 28;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Get filename index
	const u8* file_name_index_ptr = &index[file_entry_index_size];

	// Getting file information
	std::vector<SFileInfo> file_infos;
	std::vector<SFileInfo> mask_file_infos;
	std::vector<SFileInfo> not_mask_file_infos;

	for (u32 i = 0; i < num_files; i++)
	{
		SFileInfo file_info;

		// Get filename from the filename index
		TCHAR file_name[256];
		lstrcpy(file_name, reinterpret_cast<LPCTSTR>(file_name_index_ptr));
		file_info.name = file_name;

		// Get the starting address from the index and filesize
		file_info.start = *reinterpret_cast<u32*>(&index[i * 12 + 4]);
		file_info.size_cmp = *reinterpret_cast<u32*>(&index[i * 12 + 8]);
		file_info.size_org = file_info.size_cmp;

		// Get the ending address
		file_info.end = file_info.start + file_info.size_cmp;

		// Add file information to list
		if (lstrcmpi(PathFindExtension(file_info.name), _T(".rc8")) == 0)
		{
			if (file_info.name[file_info.name.GetLength() - lstrlen(_T("_.rc8"))] == _T('_'))
			{
				mask_file_infos.push_back(file_info);
			}
			else
			{
				not_mask_file_infos.push_back(file_info);
			}
		}
		else
		{
			file_infos.push_back(file_info);
		}

		file_name_index_ptr += lstrlen(file_name) + 1;
	}

	// Sort by filename
	std::sort(file_infos.begin(), file_infos.end(), CArcFile::CompareForFileInfo);

	// Get file information from the masked image
	for (auto& mask_file_info : mask_file_infos)
	{
		// Get the name of the file to be created
		TCHAR rct_name[_MAX_FNAME];
		lstrcpy(rct_name, mask_file_info.name);
		rct_name[lstrlen(rct_name) - lstrlen(_T("_.rc8"))] = _T('\0');
		PathRenameExtension(rct_name, _T(".rct"));

		// Get the file information to be created
		SFileInfo* target_file_info = archive->SearchForFileInfo(file_infos, rct_name);

		if (target_file_info != nullptr)
		{
			// Image is masked
			target_file_info->starts.push_back(mask_file_info.start);
			target_file_info->sizes_cmp.push_back(mask_file_info.size_cmp);
			target_file_info->sizes_org.push_back(mask_file_info.size_org);

			// Update progress
			archive->GetProg()->UpdatePercent(mask_file_info.size_cmp);
		}
		else
		{
			// Image is not masked
			not_mask_file_infos.push_back(mask_file_info);
		}
	}

	// Add to list view
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

/// MJO Mounting
///
/// @param archive Archive
///
bool CMajiro::MountMJO(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "MajiroObjX", 10) != 0)
		return false;

	return archive->Mount();
}

/// RC Mounting
///
/// @param archive Archive
///
bool CMajiro::MountRC(CArcFile* archive)
{
	// SJIS "˜Z’š"
	if (memcmp(archive->GetHeader(), "\x98\x5A\x92\x9A", 4) != 0)
		return false;

	return archive->Mount();
}

/// Decoding
///
/// @param archive Archive
///
bool CMajiro::Decode(CArcFile* archive)
{
	if (DecodeMJO(archive))
		return true;

	if (DecodeRC(archive))
		return true;

	return false;
}

/// MJO Decoding
///
/// @param archive Archive
///
bool CMajiro::DecodeMJO(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->format != _T("MJO"))
		return false;

	// Decoding table

	static constexpr std::array<u32, 256> key_table{{
		0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
		0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
		0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
		0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
		0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
		0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
		0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
		0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
		0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
		0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
		0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
		0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
		0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
		0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
		0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
		0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
		0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
		0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
		0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
		0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
		0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
		0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
		0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
		0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
		0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
		0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
		0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
		0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
		0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
		0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
		0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
		0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
	}};

	// Skip
	u32 offset;
	archive->SeekCur(24);
	archive->ReadU32(&offset);
	offset *= 8;

	// Get the output size
	u32 dst_size;
	archive->SeekCur(offset);
	archive->ReadU32(&dst_size);

	// Read
	std::vector<u8> dst(dst_size + 3);
	archive->Read(dst.data(), dst_size);

	// Decode
	for (size_t i = 0, j = 0; i < dst_size; i += 4)
	{
		*reinterpret_cast<u32*>(&dst[i]) ^= key_table[j++];
		j &= 255;
	}

	// Output
	archive->OpenScriptFile();
	archive->WriteFile(dst.data(), dst_size);

	return true;
}

/// RC Decoding
///
/// @param archive Archive
///
bool CMajiro::DecodeRC(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->format != _T("RCT") && file_info->format != _T("RC8"))
	{
		return false;
	}

	// rc8/rct reading
	std::vector<u8> src(file_info->size_cmp);
	archive->Read(src.data(), src.size());

	u16  bpp = (memcmp(&src[4], "8_00", 4) == 0) ? 8 : 24;
	const s32 width = *reinterpret_cast<const s32*>(&src[8]);
	const s32 height = *reinterpret_cast<const s32*>(&src[12]);
	const u32 src_data_size = *reinterpret_cast<const u32*>(&src[16]);

	size_t src_idx = 20;

	// Ensure output buffer exists
	u32             dst_size = width * height * (bpp >> 3);
	std::vector<u8> dst(dst_size);
	u32             dst2_size;
	std::vector<u8> dst2;
	u8*             dst_ptr = dst.data();

	if (bpp == 8)
	{
		// rc8
		read_bits_8(dst.data(), dst_size, &src[src_idx + 768], src_data_size, width);
	}
	else if (memcmp(&src[4], "TC00", 4) == 0)
	{
		// rct
		read_bits_24(dst.data(), dst_size, &src[src_idx], src_data_size, width);

		// Add the mask image and make the data 32-bit
		dst2_size = width * height * 4;
		dst2.resize(dst2_size);

		if (AppendMask(archive, dst2.data(), dst2_size, dst.data(), dst_size))
		{
			// Successfully appended the mask image
			bpp = 32;
			dst_size = dst2_size;
			dst_ptr = dst2.data();
		}
	}
	else if (memcmp(&src[4], "TC01", 4) == 0)
	{
		// rct(Difference information)

		// Get the base image file name
		TCHAR file_name[_MAX_FNAME];
		const u16 file_name_length = *reinterpret_cast<u16*>(&src[src_idx]);
		memcpy(file_name, &src[src_idx + 2], file_name_length);
		src_idx += 2 + file_name_length;

		// Get file info for the base image
		const SFileInfo* base_image_file_info = archive->GetFileInfo(file_name);

		if (base_image_file_info != nullptr)
		{
			// Base image file exists
			std::vector<u8> base_image(base_image_file_info->size_cmp);
			archive->SeekHed(base_image_file_info->start);
			archive->Read(base_image.data(), base_image.size());
			read_bits_24(dst.data(), dst.size(), &base_image[20], *reinterpret_cast<u32*>(&base_image[16]), width);

			// Decompress the difference image
			std::vector<u8> difference_image(dst_size);
			read_bits_24(difference_image.data(), difference_image.size(), &src[src_idx], src_data_size, width);

			// Synthesize the difference between the difference image and the base image
			for (size_t i = 0; i < dst_size; i += 3)
			{
				if (memcmp(&difference_image[i], "\x00\x00\xFF", 3) != 0)
				{
					// Red part is determined to be the transparent color in the difference image
					memcpy(&dst[i], &difference_image[i], 3);
				}
			}
		}

		// Add mask image and make the data 32-bit.
		dst2_size = width * height * 4;
		dst2.resize(dst2_size);

		if (AppendMask(archive, dst2.data(), dst2_size, dst.data(), dst_size))
		{
			// Success in adding the mask image
			bpp = 32;
			dst_size = dst2_size;
			dst_ptr = dst2.data();
		}
	}
	else
	{
		archive->OpenFile();
		archive->WriteFile(src.data(), src.size());

		return true;
	}

	// Output
	CImage image;
	image.Init(archive, width, height, bpp, &src[src_idx], 768);
	image.WriteReverse(dst_ptr, dst_size);

	return true;
}

/// RCT Extraction
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
///
void CMajiro::read_bits_24(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width)
{
	size_t src_idx = 0;
	size_t dst_idx = 0;

	memcpy(dst, src, 3);

	src_idx += 3;
	dst_idx += 3;

	while (src_idx < src_size && dst_idx < dst_size)
	{
		const u8 code = src[src_idx++];

		if (code <= 0x7F)
		{
			u32 length;
			if (code == 0x7F)
			{
				length = *reinterpret_cast<const u16*>(&src[src_idx]) + 128;
				src_idx += 2;
			}
			else
			{
				length = code + 1;
			}

			length *= 3;

			if (src_idx + length > src_size)
			{
				length = src_size - src_idx;
			}

			if (dst_idx + length > dst_size)
			{
				length = dst_size - dst_idx;
			}

			memcpy(&dst[dst_idx], &src[src_idx], length);

			src_idx += length;
			dst_idx += length;
		}
		else
		{
			u32 length = code & 0x03;
			if (length == 3)
			{
				length = *reinterpret_cast<const u16*>(&src[src_idx]) + 4;
				src_idx += 2;
			}
			else
			{
				length++;
			}

			u32 back = (code >> 2) - 32;
			if (back < 6)
			{
				back++;
			}
			else if (back <= 12)
			{
				back = width + back - 9;
			}
			else if (back <= 19)
			{
				back = width * 2 + back - 16;
			}
			else if (back <= 26)
			{
				back = width * 3 + back - 23;
			}
			else
			{
				back = width * 4 + back - 29;
			}

			length *= 3;
			back *= 3;

			if (dst_idx + length > dst_size)
			{
				length = dst_size - dst_idx;
			}

			for (size_t i = 0; i < length; i++)
			{
				dst[dst_idx + i] = dst[dst_idx - back + i];
			}

			dst_idx += length;
		}
	}
}

/// RC8 Extraction
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Compressed data
/// @param src_size Compressed data size
/// @param width    Width
///
void CMajiro::read_bits_8(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width)
{
	size_t src_idx = 0;
	size_t dst_idx = 0;

	dst[dst_idx++] = src[src_idx++];

	while (src_idx < src_size && dst_idx < dst_size)
	{
		const u8 code = src[src_idx++];
		if (code <= 0x7F)
		{
			u32 length;

			if (code == 0x7F)
			{
				length = *reinterpret_cast<const u16*>(&src[src_idx]) + 128;
				src_idx += 2;
			}
			else
			{
				length = code + 1;
			}

			if (src_idx + length > src_size)
			{
				length = src_size - src_idx;
			}

			if (dst_idx + length > dst_size)
			{
				length = dst_size - dst_idx;
			}

			memcpy(&dst[dst_idx], &src[src_idx], length);

			src_idx += length;
			dst_idx += length;
		}
		else
		{
			u32 length = code & 0x07;
			if (length == 7)
			{
				length = *reinterpret_cast<const u16*>(&src[src_idx]) + 10;
				src_idx += 2;
			}
			else
			{
				length += 3;
			}

			u32 back = (code >> 3) - 16;
			if (back < 4)
			{
				back++;
			}
			else if (back <= 10)
			{
				back = width + back - 7;
			}
			else
			{
				back = width * 2 + back - 13;
			}

			if (dst_idx + length > dst_size)
			{
				length = dst_size - dst_idx;
			}

			for (size_t i = 0; i < length; i++)
			{
				dst[dst_idx + i] = dst[dst_idx - back + i];
			}

			dst_idx += length;
		}
	}
}

/// Adds a mask image to the given 24-bit data, converting it to 32-bit data.
///
/// @param archive  Archive
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      24-bit data
/// @param src_size Data size
///
bool CMajiro::AppendMask(CArcFile* archive, u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->starts.empty())
	{
		// Mask image doesn't exist.
		return false;
	}

	// Read image mask
	std::vector<u8> mask_src(file_info->sizes_cmp[0]);
	archive->SeekHed(file_info->starts[0]);
	archive->Read(mask_src.data(), mask_src.size());

	// Get header information
	SRCHeader mask_header;
	std::memcpy(&mask_header, mask_src.data(), sizeof(SRCHeader));

	// Decompress masked image
	const u32 mask_dst_size = static_cast<u32>(mask_header.width * mask_header.height);
	std::vector<u8> mask_dst(mask_dst_size);
	read_bits_8(mask_dst.data(), mask_dst_size, &mask_src[20 + 768], mask_header.data_size, mask_header.width);

	// Make file
	size_t src_idx = 0;
	size_t mask_idx = 0;
	size_t dst_idx = 0;

	while (src_idx < src_size && mask_idx < mask_dst_size && dst_idx < dst_size)
	{
		memcpy(&dst[dst_idx], &src[src_idx], 3);

		dst[dst_idx + 3] = ~mask_dst[mask_idx];

		src_idx += 3;
		mask_idx += 1;
		dst_idx += 4;
	}

	return true;
}
