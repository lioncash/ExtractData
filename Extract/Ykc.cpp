#include "StdAfx.h"
#include "Ykc.h"

bool CYkc::Mount(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "YKC001", 6) != 0)
		return false;

	// Get the index size and the offset to the index
	u32 index_offset;
	u32 index_size;
	archive->SeekHed(0x10);
	archive->ReadU32(&index_offset);
	archive->ReadU32(&index_size);

	// Get the index
	std::vector<u8> index(index_size);
	archive->SeekHed(index_offset);
	archive->Read(index.data(), index.size());

	// Get the offset of the filename, and the offset to the index file name
	const u32 file_name_index_offset = *(u32*)&index[0];
	const u32 file_name_index_size = index_offset - file_name_index_offset;

	// Get the filename index
	std::vector<u8> file_name_index(file_name_index_size);
	archive->SeekHed(file_name_index_offset);
	archive->Read(file_name_index.data(), file_name_index.size());

	// Get file information
	for (u32 i = 0, j = 0; i < index_size; i += 20)
	{
		// Get the length of the filename
		const u32 file_name_length = *(u32*)&index[i + 4];

		// Get the filename
		TCHAR file_name[_MAX_FNAME];
		lstrcpy(file_name, (LPCTSTR)&file_name_index[j]);
		j += file_name_length;

		// Get file information
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = *(u32*)&index[i + 8];
		file_info.sizeCmp = *(u32*)&index[i + 12];
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.end = file_info.start + file_info.sizeCmp;

		archive->AddFileInfo(file_info);
	}

	return true;
}

bool CYkc::Decode(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "YKC001", 6) != 0)
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
	std::vector<u8> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	if (memcmp(src.data(), "YKS001", 6) == 0)
	{
		// YKS001

		// Get the offset of the text portion
		const u32 text_offset = *(u32*)&src[0x20];

		// Decode the text portion
		for (size_t i = text_offset; i < src.size(); i++)
		{
			src[i] ^= 0xAA;
		}

		// Output
		archive->OpenScriptFile();
		archive->WriteFile(src.data(), src.size());
	}
	else
	{
		// Other
		archive->OpenFile();
		archive->WriteFile(src.data(), src.size());
	}

	return true;
}

bool CYkc::DecodeYKG(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("YKG"))
		return false;

	// Read the YKG file
	std::vector<u8> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	if (memcmp(src.data(), "YKG000", 6) == 0)
	{
		// YKG000

		// Fix the PNG header
		memcpy(&src[0x41], "PNG", 3);

		// Output
		archive->OpenFile(_T(".png"));
		archive->WriteFile(&src[0x40], src.size() - 0x40);
	}
	else
	{
		// Other
		archive->OpenFile();
		archive->WriteFile(src.data(), src.size());
	}

	return true;
}
