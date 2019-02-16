#include "StdAfx.h"
#include "Extract/Baldr.h"
#include "ArcFile.h"

namespace
{

struct SPACFileInfo
{
	char filename[64];
	u32  offset;
	u32  file_size;
	u32  compressed_file_size;
};

} // Anonymous namespace

/// Mounting
///
/// @param archive Archive
///
bool CBaldr::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pac"))
		return false;

	if (memcmp(archive->GetHeader(), "PAC", 3) != 0)
		return false;

	if (memcmp(&archive->GetHeader()[72], "\0\0\0\0", 4) != 0)
		return false;

	// Get filecount
	u32 num_files;
	archive->Seek(4, FILE_BEGIN);
	archive->ReadU32(&num_files);

	// Get flags
	u32 flags;
	archive->ReadU32(&flags);

	// Get compressed formats
	YCString format;
	switch (flags)
	{
	case 0: // No compression
		break;

	case 1: // LZSS
		format = _T("LZ");
		break;

	case 2: // Unknown
		break;

	case 3: // ZLIB
		format = _T("zlib");
		break;
	}

	// Get index
	std::vector<SPACFileInfo> file_indices(num_files);
	archive->Read(file_indices.data(), sizeof(SPACFileInfo) * num_files);

	// Get file info
	for (const auto& entry : file_indices)
	{
		// Get filename
    char filename[65];
		memcpy(filename, entry.filename, 64);
		filename[64] = '\0';

		if (strlen(filename) <= 4)
		{
			archive->SeekHed();
			return false;
		}

		// Add to listview
		SFileInfo file_info;
		file_info.name = filename;
		file_info.size_org = entry.file_size;
		file_info.size_cmp = entry.compressed_file_size;
		file_info.start = entry.offset;
		file_info.end = file_info.start + file_info.size_cmp;
		file_info.format = format;

		archive->AddFileInfo(file_info);
	}

	return true;
}
