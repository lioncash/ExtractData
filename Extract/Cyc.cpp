#include "StdAfx.h"
#include "../Image.h"
#include "Cyc.h"

namespace
{
bool HasValidHeaderID(const CArcFile* archive)
{
	return std::memcmp(&archive->GetHeader()[48], "PACKTYPE=", 9) == 0;
}
} // Anonymous namespace

bool CCyc::Mount(CArcFile* archive)
{
	if (MountGpk(archive))
		return true;
	if (MountVpk(archive))
		return true;
	if (MountDwq(archive))
		return true;
	if (MountWgq(archive))
		return true;
	if (MountVaw(archive))
		return true;
	if (MountXtx(archive))
		return true;
	if (MountFxf(archive))
		return true;

	return false;
}

bool CCyc::MountGpk(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".gpk"))
		return false;
	if (!HasValidHeaderID(archive))
		return false;

	// Get gtb file path
	TCHAR gtb_path[MAX_PATH];
	lstrcpy(gtb_path, archive->GetArcPath());
	PathRenameExtension(gtb_path, _T(".gtb"));

	// Open gtb file
	CFile gtb_file;
	if (gtb_file.Open(gtb_path, CFile::FILE_READ) == INVALID_HANDLE_VALUE)
		return false;

	// Number of files in the gtb file
	u32 num_files;
	gtb_file.Read(&num_files, sizeof(u32));

	// Determine table size
	const u32 table_size = num_files * 4;

	// Get the gtb file filename from the table
	std::vector<u32> file_name_length_table(num_files);
	gtb_file.Read(file_name_length_table.data(), table_size);

	// Get the table offset of the gtb file
	std::vector<u32> offset_table(num_files);
	gtb_file.Read(offset_table.data(), table_size);

	// Determine the size of the filename table
	const u32 file_name_table_size = gtb_file.GetFileSize() - 4 - table_size * 2;

	// Get the gtb file from the filename table
	std::vector<char> file_name_table(file_name_table_size);
	gtb_file.Read(file_name_table.data(), file_name_table.size());

	const char* file_name_table_ptr = file_name_table.data();

	for (size_t i = 0; i < num_files; i++)
	{
		SFileInfo file_info;
		file_info.name = file_name_table_ptr;
		file_info.name += _T(".dwq"); // dwq is archived within the gtb
		file_info.start = offset_table[i];
		file_info.end = (i + 1 == num_files) ? archive->GetArcSize() : offset_table[i + 1]; // End-of-file position is the very end of the gpk file
		file_info.sizeCmp = file_info.end - file_info.start;
		file_info.sizeOrg = file_info.sizeCmp;
		archive->AddFileInfo(file_info);

		file_name_table_ptr += file_info.name.GetLength() - 3;
	}

	return true;
}

bool CCyc::MountVpk(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".vpk"))
		return false;
	if (!HasValidHeaderID(archive))
		return false;

	// Get vtb file path
	TCHAR vtb_path[MAX_PATH];
	lstrcpy(vtb_path, archive->GetArcPath());
	PathRenameExtension(vtb_path, _T(".vtb"));
	
	// Open vtb file
	CFile vtb_file;
	if (vtb_file.Open(vtb_path, CFile::FILE_READ) == INVALID_HANDLE_VALUE)
		return false;

	// Read vtb file
	const size_t vtb_size = vtb_file.GetFileSize();
	std::vector<u8> vtb(vtb_size);
	vtb_file.Read(vtb.data(), vtb.size());

	const u8* vtb_ptr = vtb.data();

	while (true)
	{
		char file_name[12];
		memcpy(file_name, vtb_ptr, 8);
		file_name[8] = '\0';

		SFileInfo file_info;
		file_info.name = file_name;
		file_info.name += _T(".vaw");
		file_info.start = *reinterpret_cast<const u32*>(&vtb_ptr[8]);
		file_info.end = *reinterpret_cast<const u32*>(&vtb_ptr[20]);
		file_info.sizeCmp = file_info.end - file_info.start;
		file_info.sizeOrg = file_info.sizeCmp;
		archive->AddFileInfo(file_info);

		vtb_ptr += 12;
		if (vtb_ptr[0] == _T('\0'))
			break;
	}

	return true;
}

bool CCyc::MountDwq(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".dwq"))
		return false;
	if (!HasValidHeaderID(archive))
		return false;

	return archive->Mount();
}

bool CCyc::MountWgq(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".wgq"))
		return false;
	if (!HasValidHeaderID(archive))
		return false;

	return archive->Mount();
}

bool CCyc::MountVaw(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".vaw"))
		return false;
	if (!HasValidHeaderID(archive))
		return false;

	return archive->Mount();
}

bool CCyc::MountXtx(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".xtx"))
		return false;

	return archive->Mount();
}

bool CCyc::MountFxf(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".fxf"))
		return false;

	return archive->Mount();
}

bool CCyc::Decode(CArcFile* archive)
{
	if (DecodeDwq(archive))
		return true;
	if (DecodeWgq(archive))
		return true;
	if (DecodeVaw(archive))
		return true;
	if (DecodeXtx(archive))
		return true;
	if (DecodeFxf(archive))
		return true;

	return false;
}

bool CCyc::DecodeDwq(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("DWQ"))
		return false;
	if (!HasValidHeaderID(archive))
		return false;

	// Read
	std::vector<u8> buffer(file_info->sizeCmp);
	archive->Read(buffer.data(), buffer.size());

	// JPEG + MASK(PACKBMP)
	if (memcmp(buffer.data(), "JPEG+MASK", 9) == 0)
	{
		archive->OpenFile(_T(".jpg"));
		archive->WriteFile(&buffer[64], *reinterpret_cast<const u32*>(&buffer[32]));

		//CJpg jpg;
		//u32 dst_size;
		//u8* dst = jpg.Decomp(&buffer[64], *reinterpret_cast<const u32*>(&buffer[32]), &dst_size);

		//u32 maskOfs = 64 + *reinterpret_cast<const u32*>(&buffer[32]);

		//// Get BMP header
		//const auto* file_header = reinterpret_cast<const BITMAPFILEHEADER*>(&buffer[maskOfs]);
		//const auto* info_header = reinterpret_cast<const BITMAPINFOHEADER*>(&buffer[maskOfs+14]);

		//// Ensure output buffer
		//const u32 mask_size = file_header->bfSize - file_header->bfOffBits;
		//std::vector<u8> mask(mask_size);

		//// RLE Decompression
		//DecompRLE(mask.data(), &buffer[maskOfs + file_header->bfOffBits], info_header->biWidth, info_header->biHeight);

		//// Alpha Blend
		//jpg.AlphaBlend(dst, mask.data(), 0x00);

		//// Output
		//CImage image;
		//image.Init(archive, jpg.GetWidth(), jpg.GetHeight(), 24);
		//image.WriteReverse(dst, dst_size);
	}
	// JPEG
	else if (memcmp(buffer.data(), "JPEG", 4) == 0)
	{
		archive->OpenFile(_T(".jpg"));
		archive->WriteFile(&buffer[64], *reinterpret_cast<u32*>(&buffer[32]));
	}
	// BMP(RGB)
	else if (memcmp(buffer.data(), "BMP", 3) == 0)
	{
		// Get BMP Header
		const auto* file_header = reinterpret_cast<const BITMAPFILEHEADER*>(&buffer[64]);
		const auto* info_header = reinterpret_cast<const BITMAPINFOHEADER*>(&buffer[78]);

		// BGR has to sort along with RGB
		if (info_header->biBitCount == 24)
			RGBtoBGR(&buffer[64 + 54], info_header->biWidth, info_header->biHeight);

		// Deviation of the size adjustment
		u32 dst_size = file_header->bfSize - file_header->bfOffBits;
		if (((info_header->biWidth * (info_header->biBitCount >> 3) + 3) & 0xFFFFFFFC) * info_header->biHeight != info_header->biSizeImage)
			dst_size -= 2;

		// Output
		CImage image;
		image.Init(archive, info_header->biWidth, info_header->biHeight, info_header->biBitCount, &buffer[64 + 54], file_header->bfOffBits - 54);
		image.WriteReverse(&buffer[64 + file_header->bfOffBits], dst_size);
	}
	// PACKBMP
	else if (memcmp(buffer.data(), "PACKBMP", 7) == 0)
	{
		// Get BMP Header
		const auto* file_header = reinterpret_cast<const BITMAPFILEHEADER*>(&buffer[64]);
		const auto* info_header = reinterpret_cast<const BITMAPINFOHEADER*>(&buffer[78]);

		// Ensure output buffer
		const u32 dst_size = file_header->bfSize - file_header->bfOffBits;
		std::vector<u8> dst(dst_size);

		// RLE Decompression
		DecompRLE(dst.data(), &buffer[64 + file_header->bfOffBits], info_header->biWidth, info_header->biHeight);

		// Output
		CImage image;
		image.Init(archive, info_header->biWidth, info_header->biHeight, info_header->biBitCount, &buffer[64 + 54], file_header->bfOffBits - 54);
		image.WriteReverse(dst.data(), dst_size - 1); // Dummy output data
	}
	// BMP
	else if (memcmp(&buffer[64], "BM", 2) == 0)
	{
		// Get BMP Header
		const auto* file_header = reinterpret_cast<const BITMAPFILEHEADER*>(&buffer[64]);
		const auto* info_header = reinterpret_cast<const BITMAPINFOHEADER*>(&buffer[78]);

		if (memcmp(&buffer[48], "PACKTYPE=1", 10) == 0)
		{
			// Ensure output buffer
			const u32 dst_size = file_header->bfSize - file_header->bfOffBits;
			std::vector<u8> dst(dst_size);

			// RLE Decompression
			DecompRLE(dst.data(), &buffer[64 + file_header->bfOffBits], info_header->biWidth, info_header->biHeight);

			// Output
			CImage image;
			image.Init(archive, info_header->biWidth, info_header->biHeight, info_header->biBitCount, &buffer[64 + 54], file_header->bfOffBits - 54);
			image.WriteReverse(dst.data(), dst_size - 1); // Dummy output data
		}
		else
		{
			// Output
			CImage image;
			image.Init(archive, info_header->biWidth, info_header->biHeight, info_header->biBitCount, &buffer[64 + 54], file_header->bfOffBits - 54);
			image.WriteReverse(&buffer[64 + file_header->bfOffBits], file_header->bfSize - file_header->bfOffBits);
		}
	}
	// Other
	else
	{
		archive->OpenFile(_T(".dwq"));
		archive->WriteFile(buffer.data(), buffer.size());
	}

	return true;
}

bool CCyc::DecodeWgq(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("WGQ"))
		return false;
	if (!HasValidHeaderID(archive))
		return false;

	// Read
	std::vector<u8> buffer(file_info->sizeCmp);
	archive->Read(buffer.data(), buffer.size());

	// Output
	archive->OpenFile(_T(".ogg"));
	archive->WriteFile(&buffer[64], file_info->sizeCmp);

	return true;
}

bool CCyc::DecodeVaw(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("VAW"))
		return false;
	if (!HasValidHeaderID(archive))
		return false;

	// Read
	std::vector<u8> buffer(file_info->sizeCmp);
	archive->Read(buffer.data(), buffer.size());

	// ogg
	if (memcmp(&buffer[108], "OggS", 4) == 0)
	{
		archive->OpenFile(_T(".ogg"));
		archive->WriteFile(&buffer[108], file_info->sizeCmp - 108);
	}
	// wav
	else
	{
		archive->OpenFile(_T(".wav"));
		archive->WriteFile(&buffer[64], file_info->sizeCmp - 64);
	}

	return true;
}

bool CCyc::DecodeXtx(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("XTX"))
		return false;

	// Read
	std::vector<u8> buffer(file_info->sizeCmp);
	archive->Read(buffer.data(), buffer.size());

	// Output
	archive->OpenScriptFile();
	archive->WriteFile(buffer.data(), buffer.size());

	return true;
}

bool CCyc::DecodeFxf(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("FXF"))
		return false;

	// Read
	std::vector<u8> buffer(file_info->sizeCmp);
	archive->Read(buffer.data(), buffer.size());

	// Decryption
	for (size_t i = 0; i < buffer.size(); i++)
		buffer[i] ^= 0xFF;

	// Output
	archive->OpenScriptFile();
	archive->WriteFile(buffer.data(), buffer.size());

	return true;
}

void CCyc::DecompRLE(u8* dst, const u8* src, s32 width, s32 height)
{
	// Ensure a 1 row/line buffer
	std::vector<u8> data_width(width);
	std::vector<u8> predataWidth(width);

	for (int i = 0; i < height; i++)
	{
		// Pointer to the line buffer
		u8* data_width_ptr = data_width.data();

		// Continue to process the rows
		for (int j = 0; j < width; )
		{
			// Get sequential input 
			const u8 data = *src++;

			// RLE Compression if data is zero
			if (data == 0)
			{
				// Following input data is written sequentially
				const u8 len = *src++;

				// RLE Decompression
				std::fill(data_width_ptr, data_width_ptr + len, data);

				// Proceed with decompressed length
				j += len;
			}
			// Entered into the dictionary as it is otherwise non-compressed data
			else
			{
				*data_width_ptr++ = data;
				j++;
			}
		}

		// Decoded using the data of one row of data before this line
		for (int j = 0; j < width; j++)
		{
			data_width[j] ^= predataWidth[j];
			// For use in decoding of the next line, save the data in this row
			predataWidth[j] = data_width[j];
		}

		// Output the decoded data to the output buffer.
		data_width_ptr = data_width.data();
		std::copy(data_width_ptr, data_width_ptr + width, dst);
	}
}

void CCyc::RGBtoBGR(u8* buffer, s32 width, s32 height)
{
	// Seek the width
	const s32 lwidth = width * 3;
	// Compute padding width
	const s32 lwidth_rev = (lwidth + 3) & 0xFFFFFFFC;

	for (int y = 0; y < height; y++)
	{
		// Replace the R and B
		for (int x = 0; x < lwidth; x += 3)
		{
			std::swap(buffer[0], buffer[2]);
			buffer += 3;
		}

		// Skip padding
		for (int x = lwidth; x < lwidth_rev; x++)
		{
			buffer++;
		}
	}
}
