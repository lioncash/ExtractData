#include "StdAfx.h"
#include "MeltyBlood.h"

// Function that gets file information from MELTY BLOOD *.data files
bool CMeltyBlood::Mount(CArcFile* archive)
{
	if (archive->GetHed()[0] != 0x00 && archive->GetHed()[0] != 0x01)
		return false;

	if (archive->GetArcName().Left(5) != _T("data0") || archive->GetArcExten() != _T(".p"))
		return false;

	constexpr u32 decryption_key = 0xE3DF59AC;

	// Get file count
	u32 num_files;
	archive->Seek(4, FILE_BEGIN);
	archive->ReadU32(&num_files);
	num_files ^= decryption_key;

	// Get index size from file count
	const u32 index_size = num_files * 68;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index_size);

	const u8* index_ptr = index.data();

	for (size_t i = 0; i < num_files; i++)
	{
		// Get file name
		std::array<TCHAR, 60> file_name;
		memcpy(file_name.data(), index_ptr, file_name.size());
		
		// Decrypt filename
		for (size_t j = 0; j < 59; j++)
		{
			file_name[j] ^= i * j * 3 + 61;
		}

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name.data();
		file_info.sizeOrg = *reinterpret_cast<const u32*>(&index_ptr[64]) ^ decryption_key;
		file_info.sizeCmp = file_info.sizeOrg;
		file_info.start = *reinterpret_cast<const u32*>(&index_ptr[60]);
		file_info.end = file_info.start + file_info.sizeOrg;
		file_info.title = _T("MeltyBlood");
		archive->AddFileInfo(file_info);

		index_ptr += 68;
	}

	return true;
}

// Extraction function
bool CMeltyBlood::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("MeltyBlood"))
		return false;

	// Create output file
	archive->OpenFile();

	// Decodes data range 0x00~0x2173, Output
	Decrypt(archive);

	// Output earlier than 0x2173
	if (file_info->sizeOrg > 0x2173)
		archive->ReadWrite(file_info->sizeOrg - 0x2173);

	return true;
}

// Data decryption function
void CMeltyBlood::Decrypt(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Ensure buffer
	const size_t buffer_size = (file_info->sizeOrg < 0x2173) ? file_info->sizeOrg : 0x2173;
	std::vector<u8> buffer(buffer_size);

	archive->Read(buffer.data(), buffer_size);

	// Decryption
	const int key_length = file_info->name.GetLength();
	
	for (size_t i = 0; i < buffer_size; i++)
	{
		buffer[i] ^= file_info->name[i % key_length] + i + 3;
	}

	archive->WriteFile(buffer.data(), buffer_size);
}
