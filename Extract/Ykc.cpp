#include "stdafx.h"
#include "Ykc.h"

bool CYkc::Mount(CArcFile* archive)
{
	if (memcmp(archive->GetHed(), "YKC001", 6) != 0)
		return false;

	// Get the index size and the offset to the index
	DWORD index_offset;
	DWORD index_size;
	archive->SeekHed(0x10);
	archive->Read(&index_offset, 4);
	archive->Read(&index_size, 4);

	// Get the index
	YCMemory<BYTE> index(index_size);
	archive->SeekHed(index_offset);
	archive->Read(&index[0], index_size);

	// Get the offset of the filename, and the offset to the index file name
	const DWORD file_name_index_offset = *(LPDWORD)&index[0];
	const DWORD file_name_index_size = index_offset - file_name_index_offset;

	// Get the filename index
	YCMemory<BYTE> file_name_index(file_name_index_size);
	archive->SeekHed(file_name_index_offset);
	archive->Read(&file_name_index[0], file_name_index_size);

	// Get file information
	for (DWORD i = 0, j = 0; i < index_size; i += 20)
	{
		// Get the length of the filename
		const DWORD file_name_length = *(LPDWORD)&index[i + 4];

		// Get the filename
		TCHAR file_name[_MAX_FNAME];
		lstrcpy(file_name, (LPCTSTR)&file_name_index[j]);
		j += file_name_length;

		// Get file information
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = *(LPDWORD)&index[i + 8];
		file_info.sizeCmp = *(LPDWORD)&index[i + 12];
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.end = file_info.start + file_info.sizeCmp;

		archive->AddFileInfo(file_info);
	}

	return true;
}

bool CYkc::Decode(CArcFile* archive)
{
	if (memcmp(archive->GetHed(), "YKC001", 6) != 0)
		return false;

	if (DecodeYKS(archive))
		return true;

	if (DecodeYKG(archive))
		return true;

	return false;
}

bool CYkc::DecodeYKS(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->format != _T("YKS"))
		return false;

	// Read the YKS file
	YCMemory<BYTE> src(file_info->sizeCmp);
	archive->Read(&src[0], file_info->sizeCmp);

	if (memcmp(&src[0], "YKS001", 6) == 0)
	{
		// YKS001

		// Get the offset of the text portion
		const DWORD text_offset = *(LPDWORD)&src[0x20];

		// Decode the text portion
		for (size_t i = text_offset; i < file_info->sizeCmp; i++)
		{
			src[i] ^= 0xAA;
		}

		// Output
		archive->OpenScriptFile();
		archive->WriteFile(&src[0], file_info->sizeCmp);
	}
	else
	{
		// Other
		archive->OpenFile();
		archive->WriteFile(&src[0], file_info->sizeCmp);
	}

	return true;
}

bool CYkc::DecodeYKG(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("YKG"))
		return false;

	// Read the YKG file
	YCMemory<BYTE> src(file_info->sizeCmp);
	archive->Read(&src[0], file_info->sizeCmp);

	if (memcmp(&src[0], "YKG000", 6) == 0)
	{
		// YKG000

		// Fix the PNG header
		memcpy(&src[0x41], "PNG", 3);

		// Output
		archive->OpenFile(_T(".png"));
		archive->WriteFile(&src[0x40], file_info->sizeCmp - 0x40);
	}
	else
	{
		// Other
		archive->OpenFile();
		archive->WriteFile(&src[0], file_info->sizeCmp);
	}

	return true;
}
