#include "StdAfx.h"
#include "Extract/TH2.h"

#include "ArcFile.h"
#include "Arc/LZSS.h"
#include "Image.h"
#include "Image/Tga.h"

/// Mounting
bool CTH2::Mount(CArcFile* archive)
{
	if (MountKCAP(archive))
		return true;

	if (MountLAC(archive))
		return true;

	if (MountDpl(archive))
		return true;

	if (MountWMV(archive))
		return true;

	return false;
}

/// KCAP Mounting
bool CTH2::MountKCAP(CArcFile* archive)
{
	if (archive->GetArcExten().CompareNoCase(_T(".pak")) != 0)
		return false;

	if (memcmp(archive->GetHeader(), "KCAP", 4) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->SeekHed(4);
	archive->ReadU32(&num_files);

	// Get index size from file count
	const u32 index_size = num_files * 36;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index_size);

	u32 index_ptr = 0;
	for (u32 i = 0; i < num_files; i++)
	{
		const u32 type = *reinterpret_cast<const u32*>(&index[index_ptr + 0]);

		// Skip garbage files
		if (type == 0xCCCCCCCC)
		{
			index_ptr += 36;
			continue;
		}

		// Get filename
		char file_name[25];
		memcpy(file_name, &index[index_ptr + 4], 24);
		file_name[24] = '\0';

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = *reinterpret_cast<const u32*>(&index[index_ptr + 28]);
		file_info.size_cmp = *reinterpret_cast<const u32*>(&index[index_ptr + 32]);
		file_info.size_org = file_info.size_cmp;
		file_info.end = file_info.start + file_info.size_cmp;
		file_info.title = _T("TH2");

		if (type == 1)
		{
			file_info.format = _T("LZ");
		}

		archive->AddFileInfo(file_info);

		index_ptr += 36;
	}

	return true;
}

/// LAC Mounting
bool CTH2::MountLAC(CArcFile* archive)
{
	if (archive->GetArcExten().CompareNoCase(_T(".pak")) != 0)
		return false;

	if (memcmp(archive->GetHeader(), "LAC", 3) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->SeekHed(4);
	archive->ReadU32(&num_files);

	// Get index size from file count
	const u32 index_size = num_files * 40;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	u32 index_ptr = 0;
	for (u32 i = 0; i < num_files; i++)
	{
		// Get filename
		char file_name[33];
		memcpy(file_name, &index[index_ptr + 0], 32);
		file_name[32] = '\0';
		for (size_t j = 0; j < strlen(file_name); j++)
		{
			file_name[j] ^= 0xFF;
		}

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.size_cmp = *reinterpret_cast<const u32*>(&index[index_ptr + 32]);
		file_info.size_org = file_info.size_cmp;
		file_info.start = *reinterpret_cast<const u32*>(&index[index_ptr + 36]);
		file_info.end = file_info.start + file_info.size_cmp;

		archive->AddFileInfo(file_info);

		index_ptr += 40;
	}

	return true;
}

/// Dpl mounting
bool CTH2::MountDpl(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".a"))
		return false;

	if (memcmp(archive->GetHeader(), "\x1E\xAF", 2) != 0)
		return false;

	// Get file count
	u16 num_files;
	archive->SeekHed(2);
	archive->ReadU16(&num_files);

	// Get index size from file count
	const u32 index_size = num_files << 5;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	const u32 offset = index_size + 4;
	u32 index_ptr = 0;
	for (u32 i = 0; i < num_files; i++)
	{
		// Get filename
		char file_name[25];
		memcpy(file_name, &index[index_ptr + 0], 24);
		file_name[24] = '\0';

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.size_cmp = *reinterpret_cast<const u32*>(&index[index_ptr + 24]);
		file_info.size_org = file_info.size_cmp;
		file_info.start = *reinterpret_cast<const u32*>(&index[index_ptr + 28]) + offset;
		file_info.end = file_info.start + file_info.size_cmp;

		archive->AddFileInfo(file_info);

		index_ptr += 32;
	}

	return true;
}

/// WMV Mounting
bool CTH2::MountWMV(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".wmv"))
		return false;

	if (memcmp(archive->GetHeader(), "\x00\x00\x00\x00\x00\x00\x00\x00\xA6\xD9\x00\xAA\x00\x62\xCE\x6C", 16) != 0)
		return false;

	return archive->Mount();
}

/// Decoding
bool CTH2::Decode(CArcFile* archive)
{
	if (DecodeWMV(archive))
		return true;

	if (DecodeEtc(archive))
		return true;

	return false;
}

/// WMV Decoding
bool CTH2::DecodeWMV(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (archive->GetArcExten() != _T(".wmv"))
		return false;

	if (memcmp(archive->GetHeader(), "\x00\x00\x00\x00\x00\x00\x00\x00\xA6\xD9\x00\xAA\x00\x62\xCE\x6C", 16) != 0)
		return false;

	// Output
	archive->OpenFile();
	archive->SeekCur(8);
	archive->WriteFile("\x30\x26\xB2\x75\x8E\x66\xCF\x11", 8);
	archive->ReadWrite(file_info->size_cmp - 8);
	archive->CloseFile();

	return true;
}

/// Decoding other files
bool CTH2::DecodeEtc(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->title != _T("TH2"))
		return false;

	if (file_info->format != _T("LZ") && file_info->format != _T("TGA"))
		return false;

	std::vector<u8> dst;
	u32 dst_size;

	// LZ Decoding
	if (file_info->format == _T("LZ"))
	{
		// Get input size
		const u32 src_size = file_info->size_cmp - 8;

		// Get output size
		archive->SeekCur(4);
		archive->ReadU32(&dst_size);

		// Ensure buffers exist
		std::vector<u8> src(src_size);
		dst.resize(dst_size);

		// Read
		archive->Read(src.data(), src.size());

		// LZSS Decompression
		CLZSS lzss;
		lzss.Decomp(dst.data(), dst_size, src.data(), src.size(), 4096, 4078, 3);
	}
	else
	{
		// Uncompressed
		dst_size = file_info->size_org;
		dst.resize(dst_size);
		archive->Read(dst.data(), dst_size);
	}

	const YCString file_extension = file_info->name.GetFileExt().MakeLower();
	if (file_extension == _T(".tga"))
	{
		// TGA
		CTga tga;
		tga.Decode(archive, dst.data(), dst_size);
	}
	else if (file_extension == _T(".bmp"))
	{
		// BMP
		CImage image;
		image.Init(archive, dst.data());
		image.Write(dst_size);
	}
	else
	{
		// Other
		archive->OpenFile();
		archive->WriteFile(dst.data(), dst_size);
	}

	return true;
}
