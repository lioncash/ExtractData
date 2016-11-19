#include "StdAfx.h"
#include "../../Image.h"
#include "JBP1.h"
#include "Cmv.h"

bool CCmv::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".cmv"))
		return false;

	if (memcmp(archive->GetHed(), "CMV", 3) != 0)
		return false;

	// Read Header
	BYTE header[44];
	archive->Read(header, sizeof(header));

	// Get offset
	const DWORD offset = *(DWORD*)&header[4];

	// Get index size
	const DWORD index_size = *(DWORD*)&header[4] - 44;

	// Get file count
	const DWORD file_count = *(DWORD*)&header[16] + 1;

	// Get index
	YCMemory<BYTE> index(index_size);
	DWORD          index_ptr = 0;
	archive->Read(&index[0], index_size);

	// Get archive name
	TCHAR archive_name[_MAX_FNAME];
	lstrcpy(archive_name, archive->GetArcName());
	PathRenameExtension(archive_name, _T("_"));

	// Get file info
	for (DWORD i = 0; i < file_count; i++)
	{
		TCHAR file_ext[_MAX_EXT];
		const DWORD type = *(DWORD*)&index[12];

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
		file_info.name = filename;
		file_info.sizeCmp = *(DWORD*)&index[index_ptr + 4];
		file_info.sizeOrg = *(DWORD*)&index[index_ptr + 8];
		file_info.start = *(DWORD*)&index[index_ptr + 16] + offset;
		file_info.end = file_info.start + file_info.sizeCmp;
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

	YCMemory<BYTE> src(file_info->sizeCmp);
	archive->Read(&src[0], file_info->sizeCmp);

	const LONG width = *(LPWORD)&src[0x10];
	const LONG height = *(LPWORD)&src[0x12];
	const WORD bpp = *(LPWORD)&src[0x14];
	const WORD colors = bpp >> 3;

	const DWORD dst_size = width * height * colors;
	YCMemory<BYTE> dst(dst_size);

	if (memcmp(&src[0], "JBP1", 4) == 0)
	{
		CJBP1 jbp1;
		jbp1.Decomp(&dst[0], &src[0]);

		CImage image;
		image.Init(archive, width, height, bpp);
		image.WriteReverse(&dst[0], dst_size);
	}
	else
	{
		archive->OpenFile();
		archive->WriteFile(&src[0], file_info->sizeCmp);
	}

	return true;
}
