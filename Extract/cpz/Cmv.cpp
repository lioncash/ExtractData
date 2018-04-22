#include "StdAfx.h"
#include "Extract/cpz/Cmv.h"

#include "ArcFile.h"
#include "Extract/cpz/JBP1.h"
#include "Image.h"

bool CCmv::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".cmv"))
		return false;

	if (memcmp(archive->GetHeader(), "CMV", 3) != 0)
		return false;

	// Read Header
	u8 header[44];
	archive->Read(header, sizeof(header));

	// Get offset
	const u32 offset = *reinterpret_cast<const u32*>(&header[4]);

	// Get index size
	const u32 index_size = *reinterpret_cast<const u32*>(&header[4]) - 44;

	// Get file count
	const u32 file_count = *reinterpret_cast<const u32*>(&header[16]) + 1;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Get archive name
	TCHAR archive_name[_MAX_FNAME];
	lstrcpy(archive_name, archive->GetArcName());
	PathRenameExtension(archive_name, _T("_"));

	// Get file info
	u32 index_ptr = 0;
	for (u32 i = 0; i < file_count; i++)
	{
		TCHAR file_ext[_MAX_EXT];
		const u32 type = *reinterpret_cast<const u32*>(&index[12]);

		switch (type)
		{
		case 0: // Ogg Vorbis
			lstrcpy(file_ext, _T(".ogg"));
			break;

		case 2: // JBP
			lstrcpy(file_ext, _T(".jbp"));
			break;
		}

		// Create the file name serial number
		TCHAR filename[_MAX_FNAME];
		_stprintf(filename, _T("%s%06u%s"), archive_name, i, file_ext);

		// Add to list view
		SFileInfo file_info;
		file_info.name    = filename;
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&index[index_ptr + 4]);
		file_info.sizeOrg = *reinterpret_cast<const u32*>(&index[index_ptr + 8]);
		file_info.start   = *reinterpret_cast<const u32*>(&index[index_ptr + 16]) + offset;
		file_info.end     = file_info.start + file_info.sizeCmp;
		archive->AddFileInfo(file_info);
		index_ptr += 20;
	}

	return true;
}

bool CCmv::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("JBP"))
		return false;

	std::vector<u8> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	const s32 width  = *reinterpret_cast<const u16*>(&src[0x10]);
	const s32 height = *reinterpret_cast<const u16*>(&src[0x12]);
	const u16 bpp    = *reinterpret_cast<const u16*>(&src[0x14]);
	const u16 colors = bpp >> 3;

	if (memcmp(src.data(), "JBP1", 4) == 0)
	{
		const u32 dst_size = width * height * colors;
		std::vector<u8> dst(dst_size);

		CJBP1 jbp1;
		jbp1.Decomp(dst.data(), src.data());

		CImage image;
		image.Init(archive, width, height, bpp);
		image.WriteReverse(dst.data(), dst.size());
	}
	else
	{
		archive->OpenFile();
		archive->WriteFile(src.data(), src.size());
	}

	return true;
}
