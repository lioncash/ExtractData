#include "StdAfx.h"
#include "Arc/LZSS.h"
#include "../Image.h"
#include "../Image/Tga.h"
#include "TaskForce.h"

/// Mounting
bool CTaskForce::Mount(CArcFile* archive)
{
	if (MountDat(archive))
		return true;

	if (MountTlz(archive))
		return true;

	if (MountBma(archive))
		return true;

	return false;
}

/// dat mounting
bool CTaskForce::MountDat(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".dat"))
		return false;

	if (memcmp(archive->GetHeader(), "tskforce", 8) != 0)
		return false;

	archive->SeekHed(8);

	// Get file count
	u32 num_files;
	archive->ReadU32(&num_files);

	// Get index
	std::vector<FileEntry> index(num_files);
	archive->Read(index.data(), sizeof(FileEntry)* num_files);

	for (u32 i = 0; i < num_files; i++)
	{
		SFileInfo file_info;
		file_info.name = index[i].file_name;
		file_info.sizeCmp = index[i].compressed_size;
		file_info.sizeOrg = index[i].original_size;
		file_info.start = index[i].offset;
		file_info.end = file_info.start + file_info.sizeCmp;

		if (file_info.sizeCmp != file_info.sizeOrg)
		{
			file_info.format = _T("LZ");
		}

		archive->AddFileInfo(file_info);
	}

	return true;
}

/// tlz mounting
bool CTaskForce::MountTlz(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".tsk") && archive->GetArcExten() != _T(".tfz"))
		return false;

	if (memcmp(archive->GetHeader(), "tlz", 3) != 0)
		return false;

	return archive->Mount();
}

/// bma mounting
bool CTaskForce::MountBma(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".tsz"))
		return false;

	if (memcmp(archive->GetHeader(), "bma", 3) != 0)
		return false;

	return archive->Mount();
}

/// Decoding
bool CTaskForce::Decode(CArcFile* archive)
{
	if (DecodeTlz(archive))
		return true;

	if (DecodeBma(archive))
		return true;

	if (DecodeTGA(archive))
		return true;

	return false;
}

/// tlz decoding
bool CTaskForce::DecodeTlz(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->name.GetFileExt() != _T(".tsk") && file_info->name.GetFileExt() != _T(".tfz"))
		return false;

	// Read header
	u8 header[24];
	archive->Read(header, sizeof(header));
	if (memcmp(header, "tlz", 3) != 0)
	{
		archive->SeekHed(file_info->start);
		return false;
	}

	// Get file information
	const u32 dst_size = *reinterpret_cast<const u32*>(&header[16]);
	const u32 src_size = *reinterpret_cast<const u32*>(&header[20]);

	// Read compressed data
	std::vector<u8> src(src_size);
	archive->Read(src.data(), src.size());

	// Buffer allocation for decompression
	std::vector<u8> dst(dst_size);

	// LZSS Decompression
	CLZSS lzss;
	lzss.Decomp(dst.data(), dst.size(), src.data(), src.size(), 4096, 4078, 3);

	// Output
	archive->OpenFile();
	archive->WriteFile(dst.data(), dst.size(), src.size());
	archive->CloseFile();

	return true;
}

/// BMA decoding
bool CTaskForce::DecodeBma(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->name.GetFileExt() != _T(".tsz"))
		return false;

	// Read header
	u8 header[24];
	archive->Read(header, sizeof(header));
	if (memcmp(header, "bma", 3) != 0)
	{
		archive->SeekHed(file_info->start);
		return false;
	}

	// Get file information
	const s32 width = *reinterpret_cast<const s32*>(&header[4]);
	const s32 height = *reinterpret_cast<const s32*>(&header[8]);
	const u32 dst_size = *reinterpret_cast<const u32*>(&header[16]);
	const u32 src_size = *reinterpret_cast<const u32*>(&header[20]);

	// Read compressed data
	std::vector<u8> src(src_size);
	archive->Read(src.data(), src.size());

	// Buffer allocation for decompression
	std::vector<u8> dst(dst_size);

	// LZSS Decoding
	CLZSS lzss;
	lzss.Decomp(dst.data(), dst.size(), src.data(), src.size(), 4096, 4078, 3);

	// Output
	CImage image;
	image.Init(archive, width, height, 32);
	image.WriteReverse(dst.data(), dst.size());
	image.Close();

	return true;
}

/// TGA Decoding
bool CTaskForce::DecodeTGA(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->name.GetFileExt() != _T(".tga"))
		return false;

	// Read data
	std::vector<u8> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	if (file_info->format == _T("LZ"))
	{
		// Is compressed
		std::vector<u8> dst(file_info->sizeOrg);

		// LZSS Decompression
		CLZSS lzss;
		lzss.Decomp(dst.data(), dst.size(), src.data(), src.size(), 4096, 4078, 3);

		// Output
		CTga tga;
		tga.Decode(archive, dst.data(), dst.size());
	}
	else
	{
		// Uncompressed
		CTga tga;
		tga.Decode(archive, src.data(), src.size());
	}

	return true;
}
