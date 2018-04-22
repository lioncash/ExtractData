#include "StdAfx.h"
#include "Extract/Spitan.h"

#include "ArcFile.h"

bool CSpitan::Mount(CArcFile* archive)
{
	if (MountSound(archive))
		return true;
	if (MountGraphic1(archive))
		return true;
	if (MountGraphic2(archive))
		return true;
	if (MountGraphic3(archive))
		return true;

	return false;
}

/// Function to get information from  Spitan bm0000, bv00**, k000* files.
bool CSpitan::MountSound(CArcFile* archive)
{
	if (archive->GetArcName().Left(6) != _T("bm0000") &&
	    archive->GetArcName().Left(4) != _T("bv00") &&
	    archive->GetArcName().Left(4) != _T("k000"))
	{
		return false;
	}

	archive->Seek(4, FILE_BEGIN);

	for (int i = 1; ; i++)
	{
		u8 header[16];
		archive->Read(header, sizeof(header));

		if (memcmp(header, "OggS", 4) == 0)
			break;
		if (memcmp(header, "\0\0\0\0", 4) == 0)
			break;

		// Get filename
		TCHAR file_name[_MAX_FNAME];
		_stprintf(file_name, _T("%s_%06d.ogg"), archive->GetArcName().GetString(), i);

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&header[0]);
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.start = *reinterpret_cast<const u32*>(&header[4]);
		file_info.end = file_info.start + file_info.sizeCmp;
		archive->AddFileInfo(file_info);
	}

	return true;
}

/// Function to get infomation from Spitan *.aif files
bool CSpitan::MountGraphic1(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".aif"))
		return false;
	if (memcmp(archive->GetHeader(), "\x10\0\0\0", 4) != 0)
		return false;

	for (int i = 1; ; i++)
	{
		u8 header[32];
		archive->Read(header, sizeof(header));

		// 臼NG
		if (memcmp(header, "\x89\x50NG", 4) == 0)
			break;

		// Get filename
		TCHAR file_name[_MAX_FNAME];
		_stprintf(file_name, _T("%s_%06d.png"), archive->GetArcName().GetString(), i);

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = *reinterpret_cast<const u32*>(&header[4]);
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&header[8]);
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.end = file_info.start + file_info.sizeCmp;
		archive->AddFileInfo(file_info);
	}

	return true;
}

/// Function to get information from Spitan f0* files
bool CSpitan::MountGraphic2(CArcFile* archive)
{
	bool flag = false;
	for (int i = 0; i < 5; i++)
	{
		TCHAR archive_name[_MAX_FNAME];
		_stprintf(archive_name, _T("f%02d"), i);
		if (lstrcmp(archive->GetArcName(), archive_name) == 0)
			flag = true;
	}
	
	if (!flag)
		return false;

	// Get .field file information
	std::vector<FileInfo> file_info_list;
	while (true)
	{
		u8 header[8];
		archive->Read(header, sizeof(header));

		if (memcmp(header, ".field", 6) == 0)
			break;
		if (memcmp(header, "\0\0\0\0\0\0", 6) == 0)
			break;

		FileInfo file;
		file.size = *reinterpret_cast<const u32*>(&header[0]);
		file.start = *reinterpret_cast<const u32*>(&header[4]);
		file_info_list.push_back(file);
	}

	for (size_t i = 0, num_files = 1; i < file_info_list.size(); i++)
	{
		// Get file size and relative address of the PNG file
		std::array<FileInfo, 5> files;

		archive->Seek(file_info_list[i].start + 24, FILE_BEGIN);
		for (size_t j = 0; j < 2; j++)
		{
			archive->ReadU32(&files[j].start);
			archive->ReadU32(&files[j].size);
		}

		archive->ReadU32(&files[2].start);
		// 3つ目のPNGファイルのファイルサイズは、3つ目のPNGヘッダの直前に書いてあるため、そこまで飛ぶ
		const u64 tmp_pos = archive->GetArcPointer();
		archive->Seek(file_info_list[i].start + files[2].start, FILE_BEGIN);

		int count = 0;
		while (true)
		{
			// By counting the amount of times ReadFile is called, we can determine the relative address of the third header PNG
			count++;

			u32 file_size;
			archive->ReadU32(&file_size);
			// PNGヘッダが来たら、この.fieldファイルの中のPNGファイル数は4つ(Usually 5)
			if (file_size == 0x474E5089)
			{
				files[2].size = 0;
				break;
			}
			// File size is greater than 30 bytes
			else if (file_size >= 30)
			{
				files[2].size = file_size;
				files[2].start += 4 * count;
				break;
			}
		}

		archive->Seek(tmp_pos, FILE_BEGIN);
		for (size_t j = 3; j < files.size(); j++)
		{
			archive->ReadU32(&files[j].start);
			archive->ReadU32(&files[j].size);
		}

		for (const auto& file : files)
		{
			// Skip it is the file has a size of 0
			if (file.size == 0)
				continue;

			// Get filename
			TCHAR file_name[_MAX_FNAME];
			_stprintf(file_name, _T("%s_%06zu.png"), archive->GetArcName().GetString(), num_files++);

			// Add to listview
			SFileInfo file_info;
			file_info.name = file_name;
			file_info.sizeCmp = file.size;
			file_info.sizeOrg = file_info.sizeCmp;
			file_info.start = file_info_list[i].start + file.start;
			file_info.end = file_info.start + file_info.sizeCmp;
			archive->AddFileInfo(file_info);

			archive->Seek(file_info.sizeCmp, FILE_CURRENT);
		}
	}

	return true;
}

/// Function that gets information from Spitan b0*, c0*, IGNR**** files
bool CSpitan::MountGraphic3(CArcFile* archive)
{
	bool flag = false;
	
	if (archive->GetArcName().Left(4) == _T("IGNR"))
		flag = true;
	
	for (int i = 0; i < 5; i++)
	{
		TCHAR archive_name[_MAX_FNAME];
		_stprintf(archive_name, _T("b%02d"), i);
		
		if (archive->GetArcName() == archive_name)
			flag = true;
	}
	
	for (int i = 0; i < 2; i++)
	{
		TCHAR archive_name[_MAX_FNAME];
		_stprintf(archive_name, _T("c%02d"), i);
		
		if (archive->GetArcName() == archive_name)
			flag = true;
	}
	
	if (!flag)
		return false;

	std::vector<FileInfo> file_info_list;

	// Get information from the NORI file
	while (true)
	{
		u8 header[8];
		archive->Read(header, sizeof(header));

		if (memcmp(header, "NORI", 4) == 0)
			break;
		if (memcmp(header, "\0\0\0\0", 4) == 0)
			break;

		FileInfo file;
		file.size = *reinterpret_cast<const u32*>(&header[0]);
		file.start = *reinterpret_cast<const u32*>(&header[4]);
		file_info_list.push_back(file);
	}

	for (size_t i = 0, num_files = 1; i < file_info_list.size(); i++)
	{
		// Get the number of PNG files in the NORI file
		u32 num_pngs;
		archive->Seek(file_info_list[i].start + 0x4C, FILE_BEGIN);
		archive->ReadU32(&num_pngs);

		archive->Seek(0x40 - 0x1C, FILE_CURRENT);

		for (u32 j = 0; j < num_pngs; j++)
		{
			archive->Seek(0x1C, FILE_CURRENT);

			// Get first and second file sizes
			std::array<u32, 2> file_sizes;
			for (u32& file_size : file_sizes)
			{
				archive->ReadU32(&file_size);
			}

			for (u32 file_size : file_sizes)
			{
				// Skip file if the file size is 0
				if (file_size == 0)
					continue;

				// Get filename
				TCHAR file_name[_MAX_FNAME];
				_stprintf(file_name, _T("%s_%06zu.png"), archive->GetArcName().GetString(), num_files++);

				// Add file to listview
				SFileInfo file_info;
				file_info.name = file_name;
				file_info.sizeCmp = file_size;
				file_info.sizeOrg = file_info.sizeCmp;
				file_info.start = archive->GetArcPointer();
				file_info.end = file_info.start + file_info.sizeCmp;
				archive->AddFileInfo(file_info);

				archive->Seek(file_info.sizeCmp, FILE_CURRENT);
			}
		}
	}

	return true;
}
