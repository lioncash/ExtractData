#include "StdAfx.h"
#include "Extract/InnocentGrey.h"

#include "ArcFile.h"
#include "Common.h"
#include "Image.h"

bool CInnocentGrey::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".dat") || memcmp(archive->GetHeader(), "PACKDAT.", 8) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->Seek(8, FILE_BEGIN);
	archive->ReadU32(&num_files);

	// Get index size from file count
	const size_t index_size = num_files * 48;

	// Get index
	std::vector<u8> index(index_size);
	archive->Seek(4, FILE_CURRENT);
	archive->Read(index.data(), index.size());
	const u8* index_ptr = index.data();

	for (u32 i = 0; i < num_files; i++)
	{
		// Get file name
		TCHAR file_name[32];
		memcpy(file_name, index_ptr, 32);

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = *reinterpret_cast<const u32*>(&index_ptr[32]);
		file_info.size_org = *reinterpret_cast<const u32*>(&index_ptr[40]);
		file_info.size_cmp = *reinterpret_cast<const u32*>(&index_ptr[44]);
		file_info.end = file_info.start + file_info.size_cmp;
		file_info.title = _T("InnocentGrey");
		archive->AddFileInfo(file_info);

		index_ptr += 48;
	}

	return true;
}

bool CInnocentGrey::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->title != _T("InnocentGrey") || archive->GetArcExten() != _T(".dat") || file_info->format != _T("S"))
		return false;

	if (file_info->format == _T("BMP"))
	{
		// Read
		std::vector<u8> buffer(file_info->size_org);
		archive->Read(buffer.data(), buffer.size());

		// Decryption
		for (size_t i = 0; i < buffer.size(); i++)
		{
			buffer[i] ^= 0xFF;
		}

		// Output
		CImage image;
		image.Init(archive, buffer.data());
		image.Write(buffer.size());
	}
	else
	{
		// Ensure Buffer
		size_t buffer_size = archive->GetBufSize();
		std::vector<u8> buffer(buffer_size);
		
		// Create output file
		archive->OpenFile();

		for (size_t write_size = 0; write_size != file_info->size_org; write_size += buffer_size)
		{
			// Get buffer size
			archive->SetBufSize(&buffer_size, write_size);

			// Decode output
			archive->Read(buffer.data(), buffer_size);
			for (size_t i = 0; i < buffer_size; i++)
				buffer[i] ^= 0xFF;
			archive->WriteFile(buffer.data(), buffer_size);
		}
	}

	return true;
}
