#include "StdAfx.h"
#include "Extract/Navel.h"

#include "ArcFile.h"
#include "Sound/Wav.h"

// Function that gets file information from Navel's .pac files
bool CNavel::Mount(CArcFile* archive)
{
	if (MountPac(archive))
		return true;
	if (MountWpd(archive))
		return true;

	return false;
}

bool CNavel::MountPac(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pac") || memcmp(archive->GetHeader(), "CAPF", 4) != 0)
		return false;

	// Get index size
	u32 index_size;
	archive->Seek(8, FILE_BEGIN);
	archive->ReadU32(&index_size);
	index_size -= 32;

	// Get file count
	u32 num_files;
	archive->ReadU32(&num_files);

	// Get index
	std::vector<u8> index(index_size);
	archive->Seek(16, FILE_CURRENT);
	archive->Read(index.data(), index.size());

	const u8* index_ptr = index.data();
	for (u32 i = 0; i < num_files; i++)
	{
		// Get file name
		TCHAR file_name[32];
		memcpy(file_name, &index_ptr[8], 32);

		// Add to list view
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.sizeOrg = *reinterpret_cast<const u32*>(&index_ptr[4]);
		file_info.sizeCmp = file_info.sizeOrg;
		file_info.start = *reinterpret_cast<const u32*>(&index_ptr[0]);
		file_info.end = file_info.start + file_info.sizeOrg;
		archive->AddFileInfo(file_info);

		index_ptr += 40;
	}

	return true;
}

bool CNavel::MountWpd(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".WPD") || memcmp(archive->GetHeader(), " DPW", 4) != 0)
		return false;

	return archive->Mount();
}

// Function to convert to WAV files
bool CNavel::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("WPD"))
		return false;

	// Read WPD Format
	FormatWPD wpd;
	archive->Read(&wpd, sizeof(FormatWPD));

	// Output
	CWav wav;
	wav.Init(archive, file_info->sizeOrg - 44, wpd.freq, wpd.channels, wpd.bits);
	wav.Write();

	return true;
}
