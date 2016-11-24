#include "StdAfx.h"
#include "../Arc/Zlib.h"
#include "../Image.h"
#include "Nitro.h"

bool CNitro::Mount(CArcFile* archive)
{
	if (MountPak1(archive))
		return true;
	if (MountPak2(archive))
		return true;
	if (MountPak3(archive))
		return true;
	if (MountPak4(archive))
		return true;
	if (MountPK2(archive))
		return true;
	if (MountN3Pk(archive))
		return true;
//	if (MountPck(archive))
//		return true;
	if (MountNpp(archive))
		return true;
//	if (MountNpa(archive))
//		return true;
	return false;
}

bool CNitro::MountPak1(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pak"))
		return false;
	if (memcmp(archive->GetHed(), "\x01\0\0\0", 4) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->Seek(4, FILE_BEGIN);
	archive->ReadU32(&num_files);

	// Get index size
	u32 index_size;
	archive->ReadU32(&index_size);

	// Get compressed index size
	u32 index_compsize;
	archive->ReadU32(&index_compsize);

	// Ensure buffer exists
	std::vector<u8> z_index(index_compsize);
	std::vector<u8> index(index_size);

	// Get compressed index
	archive->Read(z_index.data(), z_index.size());

	// Get index
	CZlib zlib;
	zlib.Decompress(index.data(), index_size, z_index.data(), z_index.size());

	const u32 offset = 0x10 + index_compsize;

	const u8* index_ptr = index.data();
	for (u32 i = 0; i < num_files; i++)
	{
		SFileInfo file_info;

		// Get filename
		TCHAR file_name[_MAX_FNAME];
		const u32 file_name_length = *reinterpret_cast<const u32*>(&index_ptr[0]);
		memcpy(file_name, &index_ptr[4], file_name_length);
		file_name[file_name_length] = _T('\0');
		index_ptr += 4 + file_name_length;

		file_info.start   = *reinterpret_cast<const u32*>(&index_ptr[0]) + offset; // Correction because it starts with a starting address relative to 0
		file_info.sizeOrg = *reinterpret_cast<const u32*>(&index_ptr[4]);
		const u32 cmp     = *reinterpret_cast<const u32*>(&index_ptr[12]);
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&index_ptr[16]);
		if (file_info.sizeCmp == 0)
			file_info.sizeCmp = file_info.sizeOrg;
		index_ptr += 20;

		// Add file to listview
		file_info.name = file_name;
		file_info.end = file_info.start + file_info.sizeCmp;
		if (cmp)
			file_info.format = _T("zlib");
		file_info.title = _T("Pak1");
		archive->AddFileInfo(file_info);
	}

	return true;
}

/// Function to get file information from Demonbane .pak files
///
/// @param archive Archive
///
bool CNitro::MountPak2(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pak"))
		return false;

	if (memcmp(archive->GetHed(), "\x02\0\0\0", 4) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->SeekHed(4);
	archive->ReadU32(&num_files);

	// Get index size
	u32 index_size;
	archive->ReadU32(&index_size);

	// Get compressed index size
	u32 compressed_index_size;
	archive->ReadU32(&compressed_index_size);

	// Ensure buffers exist
	std::vector<u8> compressed_index(compressed_index_size);
	std::vector<u8> index(index_size);

	// Get compressed index
	archive->SeekHed(0x114);
	archive->Read(compressed_index.data(), compressed_index.size());

	// Get index
	CZlib zlib;
	zlib.Decompress(index.data(), index_size, compressed_index.data(), compressed_index.size());

	const u32 offset = 0x114 + compressed_index_size;

	size_t index_ptr = 0;
	for (u32 i = 0; i < num_files; i++)
	{
		SFileInfo file_info;

		// Get filename
		char file_name[_MAX_FNAME];
		const u32 file_name_length = *reinterpret_cast<const u32*>(&index[index_ptr + 0]);

		memcpy(file_name, &index[index_ptr + 4], file_name_length);
		file_name[file_name_length] = '\0';

		index_ptr += 4 + file_name_length;

		// Get flags
		const u32 flags = *reinterpret_cast<const u32*>(&index[index_ptr + 12]);

		// Add to listview
		file_info.name    = file_name;
		file_info.start   = *reinterpret_cast<const u32*>(&index[index_ptr + 0]) + offset;
		file_info.sizeOrg = *reinterpret_cast<const u32*>(&index[index_ptr + 4]);
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&index[index_ptr + 16]);

		if (file_info.sizeCmp == 0)
		{
			file_info.sizeCmp = file_info.sizeOrg;
		}

		file_info.end = file_info.start + file_info.sizeCmp;

		if (flags != 0)
		{
			file_info.format = _T("zlib");
		}

		file_info.title = _T("Pak2");

		archive->AddFileInfo(file_info);

		index_ptr += 20;
	}

	return true;
}

bool CNitro::MountPak3(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pak"))
		return false;
	if (memcmp(archive->GetHed(), "\x03\0\0\0", 4) != 0)
		return false;

	// Read signature
	u8 sig[256];
	archive->Seek(4, FILE_BEGIN);
	archive->Read(sig, sizeof(sig));

	// Generate decryption key
	u32 key = 0;
	for (u8 signature_byte : sig)
	{
		if (signature_byte == 0)
			break;
		
		key *= 0x89;
		key += signature_byte;
	}

	// Read header
	u8 header[16];
	archive->Read(header, sizeof(header));

	// Get compressed index size
	const u32 index_compsize = *reinterpret_cast<const u32*>(&header[0]) ^ *reinterpret_cast<const u32*>(&header[12]);

	// Get index size
	const u32 index_size = *reinterpret_cast<const u32*>(&header[4]) ^ key;

	// Get file count
	const u32 num_files = *reinterpret_cast<const u32*>(&header[8]) ^ key;

	// Ensure buffer exists
	std::vector<u8> z_index(index_compsize);
	std::vector<u8> index(index_size);

	// Get compressed index
	archive->Read(z_index.data(), z_index.size());

	// Get index
	CZlib zlib;
	zlib.Decompress(index.data(), index_size, z_index.data(), z_index.size());

	const u32 offset = 0x114 + index_compsize;
	u32 file_end_prev = 0;

	const u8* index_ptr = index.data();
	for (u32 i = 0; i < num_files; i++)
	{
		SFileInfo file_info;

		// Get filename
		TCHAR file_name[_MAX_FNAME];
		const u32 file_name_length = *reinterpret_cast<const u32*>(&index_ptr[0]);
		memcpy(file_name, &index_ptr[4], file_name_length);
		file_name[file_name_length] = _T('\0');
		index_ptr += 4 + file_name_length;

		file_info.start = *reinterpret_cast<const u32*>(&index_ptr[0]);
		file_info.key = file_info.start ^ file_end_prev;//(i == 0) ? file_info.start : file_info.start ^ FileEnd_prev;//(archive->GetFileInfo(i-1)->end - offset);
		file_info.start ^= file_info.key;
		file_info.sizeOrg = *reinterpret_cast<const u32*>(&index_ptr[4]) ^ file_info.key;
		const u32 cmp = *reinterpret_cast<const u32*>(&index_ptr[12]) ^ file_info.key;
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&index_ptr[16]) ^ file_info.key;
		
		if (file_info.sizeCmp == 0)
			file_info.sizeCmp = file_info.sizeOrg;
		
		index_ptr += 20;

		// Add to listview
		file_info.name = file_name;
		file_info.start += offset; // Correction because it starts with a starting address relative to 0
		file_info.end = file_info.start + file_info.sizeCmp;
		if (cmp)
			file_info.format = _T("zlib");
		file_info.title = _T("Pak3");
		archive->AddFileInfo(file_info);

		file_end_prev = file_info.end - offset;
	}

	return true;
}

// Function to get file information from Demonbane .pak files.
bool CNitro::MountPak4(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pak"))
		return false;
	if (memcmp(archive->GetHed(), "\x04\0\0\0", 4) != 0)
		return false;

	// Read signature
	u8 sig[256];
	archive->Seek(4, FILE_BEGIN);
	archive->Read(sig, sizeof(sig));

	// Generate decryption key
	u32 key = 0;
	for (const u8 signature_byte : sig)
	{
		if (signature_byte == 0)
			break;
		
		key *= 0x89;
		key += signature_byte;
	}

	// Read header
	u8 header[16];
	archive->Read(header, sizeof(header));

	// Get compressed index size
	const u32 index_compsize = *reinterpret_cast<const u32*>(&header[0]) ^ *reinterpret_cast<const u32*>(&header[12]);

	// Get index size
	const u32 index_size = *reinterpret_cast<const u32*>(&header[4]) ^ key;

	// Get file count 
	const u32 num_files = *reinterpret_cast<const u32*>(&header[8]) ^ key;

	// Ensure buffers exist
	std::vector<u8> z_index(index_compsize);
	std::vector<u8> index(index_size);

	// Get compressed index
	archive->Read(z_index.data(), z_index.size());

	// Get index
	CZlib zlib;
	zlib.Decompress(index.data(), index_size, z_index.data(), z_index.size());

	const u32 offset = 0x114 + index_compsize;
	u32 file_end_prev = 0;

	const u8* index_ptr = &index[0];
	for (u32 i = 0; i < num_files; i++)
	{
		SFileInfo file_info;

		// Get filename
		TCHAR file_name[_MAX_FNAME];
		const u32 file_name_length = *reinterpret_cast<const u32*>(&index_ptr[0]);
		memcpy(file_name, &index_ptr[4], file_name_length);
		file_name[file_name_length] = _T('\0');
		index_ptr += 4 + file_name_length;

		file_info.start = *reinterpret_cast<const u32*>(&index_ptr[0]);
		file_info.key = file_info.start ^ file_end_prev;//(i == 0) ? file_info.start : file_info.start ^ FileEnd_prev;//(archive->GetFileInfo(i-1)->end - offset);
		file_info.start ^= file_info.key;
		file_info.sizeOrg = *reinterpret_cast<const u32*>(&index_ptr[4]) ^ file_info.key;
		const u32 cmp = *reinterpret_cast<const u32*>(&index_ptr[12]) ^ file_info.key;
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&index_ptr[16]) ^ file_info.key;
		if (file_info.sizeCmp == 0)
			file_info.sizeCmp = file_info.sizeOrg;
		index_ptr += 20;

		// Add to listview
		file_info.name = file_name;
		file_info.start += offset; // Correction because it starts with a starting address relative to 0
		file_info.end = file_info.start + file_info.sizeCmp;
		if (cmp)
			file_info.format = _T("zlib");
		file_info.title = _T("Pak4");
		archive->AddFileInfo(file_info);

		file_end_prev = file_info.end - offset;
	}

	return true;
}

bool CNitro::MountPK2(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".PK2") || memcmp(archive->GetHed(), "ARCV", 4) != 0)
		return false;

	archive->Seek(4, FILE_BEGIN);

	// Get compressed filename index size
	u32 name_index_compressed_size;
	archive->ReadU32(&name_index_compressed_size);

	// Get filename index size
	u32 name_index_decompressed_size;
	archive->ReadU32(&name_index_decompressed_size);
	name_index_decompressed_size &= 0x00FFFFFF;

	// Ensure buffers exist
	std::vector<u8> name_index_compressed(name_index_compressed_size);
	std::vector<u8> name_index_decompressed(name_index_decompressed_size);

	// Get compressed index filename
	archive->Read(name_index_compressed.data(), name_index_compressed.size());

	// Unzip compressed index filename
	CZlib zlib;
	zlib.Decompress(name_index_decompressed.data(), name_index_decompressed.size(),
	                name_index_compressed.data(), name_index_compressed.size());

	// Get offset index size
	u32 offset_index_size;
	archive->ReadU32(&offset_index_size);
	archive->Seek(8, FILE_CURRENT);

	// Ensure buffer exists
	std::vector<u8> offset_index(offset_index_size);

	// Get offset index
	archive->Read(offset_index.data(), offset_index.size());

	const u8* name_index_ptr = name_index_decompressed.data();
	const u8* offset_index_ptr = offset_index.data();

	for (u32 i = 0; ; i++)
	{
		// Get filename from the index
		char file_name[MAX_PATH];
		char* file_name_ptr = file_name;
		while ((*file_name_ptr++ = *name_index_ptr++) != 0x0D); // 0x0D is a termination character
		file_name_ptr[-1] = '\0'; // Substitute 0x0D for 0x00

		if (i == 0)
		{
			// Skip first file because is contains "_*.PK2"
			continue;
		}

		SFileInfo file_info;
		file_info.name    = file_name;
		file_info.start   = *reinterpret_cast<const u32*>(&offset_index_ptr[4]);
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&offset_index_ptr[8]);
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.end     = file_info.start + file_info.sizeCmp;
		archive->AddFileInfo(file_info);

		offset_index_ptr += 12;

		if (memcmp(&offset_index_ptr[4], "DATA", 4) == 0)
		{
			// Done
			break;
		}
	}

	return true;
}

bool CNitro::MountN3Pk(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pak") || memcmp(archive->GetHed(), "N3Pk", 4) != 0)
		return false;

	archive->Seek(4, FILE_BEGIN);

	// Get file count
	u32 num_files;
	archive->ReadU32(&num_files);

	archive->Seek(4, FILE_CURRENT);

	// Get index size
	const u32 index_size = 152 * num_files;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	const u8* index_ptr = index.data();
	for (u32 i = 0; i < num_files; i++)
	{
		// Get filename
		YCString file_name;
		file_name = reinterpret_cast<const char*>(&index_ptr[22]);
		file_name += _T("/");
		file_name += reinterpret_cast<const char*>(&index_ptr[86]);

		SFileInfo file_info;
		file_info.name    = file_name;
		file_info.start   = *reinterpret_cast<const u32*>(&index_ptr[0]);
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&index_ptr[4]);
		file_info.sizeOrg = *reinterpret_cast<const u32*>(&index_ptr[8]);
		file_info.end     = file_info.start + file_info.sizeCmp;
		file_info.type    = *reinterpret_cast<const u16*>(&index_ptr[18]);
		file_info.key     = index_ptr[21];
		archive->AddFileInfo(file_info);

		index_ptr += 152;
	}

	return true;
}

bool CNitro::MountPck(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pck") || memcmp(&archive->GetHed()[12], "\x00\x00\x00\x00", 4) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->ReadU32(&num_files);

	// Get offset index size
	const u32 offset_index_size = 12 * (num_files - 1);

	// Get filename index size
	u32 name_index_size;
	archive->ReadU32(&name_index_size);

	archive->Seek(4, FILE_CURRENT);

	// Ensure buffer exists
	std::vector<u8> offset_index(offset_index_size);
	std::vector<u8> name_index(name_index_size);

	// Get offset index
	archive->Read(offset_index.data(), offset_index.size());

	// Get filename index
	archive->Read(name_index.data(), name_index.size());

	const u8* offset_index_ptr = offset_index.data();
	const u8* name_index_ptr = name_index.data();

	const u32 offset = 12 + offset_index_size + name_index_size;

	for (u32 i = 0; i < num_files; i++)
	{
		// Get the filename from the index
		char file_name[MAX_PATH];
		char* file_name_ptr = file_name;
		while ((*file_name_ptr++ = *name_index_ptr++) != 0x0D); // 0x0D is a termination character
		file_name_ptr[-1] = '\0'; // substitue 0x0D for 0x00

		if (i == 0)
		{
			// Skip the first file since the name contains '.pck'
			continue;
		}

		SFileInfo file_info;
		file_info.name    = file_name;
		file_info.start   = *reinterpret_cast<const u32*>(&offset_index_ptr[0]) + offset;
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&offset_index_ptr[4]);
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.end     = file_info.start + file_info.sizeCmp;
		archive->AddFileInfo(file_info);

		offset_index_ptr += 12;
	}

	return true;
}

// “VŽgƒm“ñ’ðŒe
// This: http://www.nitroplus.co.jp/pc/lineup/into_07/
bool CNitro::MountNpp(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".npp") || memcmp(archive->GetHed(), "nitP", 4) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->Seek(4, FILE_BEGIN);
	archive->ReadU32(&num_files);

	// Get index size from the file count
	const u32 index_size = 144 * num_files;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	const u8* index_ptr = index.data();
	for (u32 i = 0; i < num_files; i++)
	{
		// Get folder and filename from the index
		TCHAR dir[64];
		TCHAR file_name[64];
		memcpy(dir, &index_ptr[16], 64);
		memcpy(file_name, &index_ptr[80], 64);

		TCHAR file_path[MAX_PATH];
		lstrcpy(file_path, dir);
		PathAppend(file_path, file_name);

		// Add to listview
		SFileInfo file_info;
		file_info.name    = file_path;
		file_info.start   = *reinterpret_cast<const u32*>(&index_ptr[0]);
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&index_ptr[4]);
		file_info.sizeOrg = *reinterpret_cast<const u32*>(&index_ptr[8]);
		file_info.end     = file_info.start + file_info.sizeCmp;
		if (index_ptr[12] != 0)
			file_info.format = _T("LZ");
		archive->AddFileInfo(file_info);

		index_ptr += 144;
	}

	return true;
}

bool CNitro::MountNpa(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".npa") || memcmp(archive->GetHed(), "NPA\x01", 4) != 0)
		return false;

	archive->Seek(25, FILE_BEGIN);

	// Get file count
	u32 num_files;
	archive->ReadU32(&num_files);

	archive->Seek(8, FILE_CURRENT);

	// Get index size
	u32 index_size;
	archive->ReadU32(&index_size);

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	const u32 offset = 41 + index_size;
	constexpr u32 base_key = 0x87654321;

	const u8* index_ptr = index.data();
	for (u32 i = 0; i < num_files; )
	{
		// Get filename
		char file_name[MAX_PATH];
		const u32 file_name_length = *reinterpret_cast<const u32*>(&index_ptr[0]);
		memcpy(file_name, &index_ptr[4], file_name_length);
		file_name[file_name_length] = '\0';
		index_ptr += 4 + file_name_length;

		// Get filename(trial version)
		//char file_name[MAX_PATH];
		//char* file_name_ptr = file_name;
		//while ((*file_name_ptr++ = *index_ptr++) != '\0');

		if (index_ptr[0] == 1)
		{
			// Folder
			index_ptr += 17;
			continue;
		}

		// Get key
		u32 key = base_key;
		const u32 key_length = static_cast<u32>(strlen(file_name));

		for (size_t j = 0; j < key_length; j++)
			key -= static_cast<u8>(file_name[j]);
		
		key *= key_length;

		SFileInfo file_info;
		file_info.name    = file_name;
		file_info.start   = *reinterpret_cast<const u32*>(&index_ptr[5]) + offset;
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&index_ptr[9]);
		file_info.sizeOrg = *reinterpret_cast<const u32*>(&index_ptr[13]);
		file_info.end     = file_info.start + file_info.sizeCmp;
		if (file_info.sizeCmp < file_info.sizeOrg)
			file_info.format = _T("zlib");
		file_info.key = key;
		archive->AddFileInfo(file_info);

		index_ptr += 17;
		i++;
	}

	return true;
}

bool CNitro::Decode(CArcFile* archive)
{
	if (DecodePak1(archive))
		return true;
	if (DecodePak3(archive))
		return true;
	if (DecodePak4(archive))
		return true;
	if (DecodePK2(archive))
		return true;
	if (DecodeN3Pk(archive))
		return true;
//	if (DecodeNpa(archive))
//		return true;
	return false;
}

bool CNitro::DecodePak1(CArcFile* archive)
{
	SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->title != _T("Pak1") && file_info->title != _T("Pak2"))
		return false;
	if (lstrcmpi(PathFindExtension(file_info->name), _T(".nps")) != 0)
		return false;

	// Create output file
	archive->OpenScriptFile();

	// zlib compressed data
	if (file_info->format == _T("zlib"))
	{
		// Ensure buffer exists
		std::vector<u8> z_buf(file_info->sizeCmp);
		std::vector<u8> dst(file_info->sizeOrg);

		// Reading
		archive->Read(z_buf.data(), z_buf.size());

		// Decompression
		CZlib zlib;
		zlib.Decompress(dst.data(), &file_info->sizeOrg, z_buf.data(), z_buf.size());

		// Output
		archive->WriteFile(dst.data(), dst.size());
	}
	else
	{
		// Uncompressed data
		archive->ReadWrite();
	}

	return true;
}

bool CNitro::DecodePak3(CArcFile* archive)
{
	SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->title != _T("Pak3"))
		return false;

	if (file_info->format == _T("BMP"))
	{
		// Reading
		std::vector<u8> buffer(file_info->sizeCmp);
		archive->Read(buffer.data(), buffer.size());
		DecryptPak3(buffer.data(), buffer.size(), 0, file_info);

		CImage image;
		image.Init(archive, buffer.data());
		image.Write(buffer.size());
	}
	else
	{
		// Create output file
		if (lstrcmpi(PathFindExtension(file_info->name), _T(".nps")) == 0)
			archive->OpenScriptFile();
		else
			archive->OpenFile();

		// zlib compressed data
		if (file_info->format == _T("zlib"))
		{
			// Ensure buffers exist
			std::vector<u8> z_buf(file_info->sizeCmp);
			std::vector<u8> dst(file_info->sizeOrg);

			// Reading
			archive->Read(z_buf.data(), z_buf.size());

			// Decompression
			CZlib zlib;
			zlib.Decompress(dst.data(), &file_info->sizeOrg, z_buf.data(), z_buf.size());

			// Output
			DecryptPak3(dst.data(), dst.size(), 0, file_info);
			archive->WriteFile(dst.data(), dst.size());
		}
		else
		{
			// Ensure buffers exist
			size_t buffer_size = archive->GetBufSize();
			std::vector<u8> buffer(buffer_size);

			for (size_t write_size = 0; write_size != file_info->sizeOrg; write_size += buffer_size)
			{
				// Adjust buffer size
				archive->SetBufSize(&buffer_size, write_size);

				// Output
				archive->Read(buffer.data(), buffer_size);
				DecryptPak3(buffer.data(), buffer_size, write_size, file_info);
				archive->WriteFile(buffer.data(), buffer_size);
			}
		}
	}

	return true;
}

bool CNitro::DecodePak4(CArcFile* archive)
{
	SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->title != _T("Pak4"))
		return false;

	if (file_info->format == _T("BMP"))
	{
		// Reading
		std::vector<u8> buffer(file_info->sizeCmp);
		archive->Read(buffer.data(), buffer.size());
		DecryptPak4(buffer.data(), buffer.size(), 0, file_info);

		CImage image;
		image.Init(archive, buffer.data());
		image.Write(buffer.size());
	}
	else
	{
		// Create output file
		if (lstrcmpi(PathFindExtension(file_info->name), _T(".nps")) == 0)
			archive->OpenScriptFile();
		else
			archive->OpenFile();

		// zlib compressed data
		if (file_info->format == _T("zlib"))
		{
			// Ensure buffer exists
			std::vector<u8> z_buf(file_info->sizeCmp);
			std::vector<u8> dst(file_info->sizeOrg);

			// Read
			archive->Read(z_buf.data(), z_buf.size());

			// Decompression
			CZlib zlib;
			zlib.Decompress(dst.data(), &file_info->sizeOrg, z_buf.data(), z_buf.size());

			// Output
			DecryptPak4(dst.data(), dst.size(), 0, file_info);
			archive->WriteFile(dst.data(), dst.size());
		}
		else
		{
			// Ensure buffer exists
			size_t buffer_size = archive->GetBufSize();
			std::vector<u8> buffer(buffer_size);

			for (size_t write_size = 0; write_size != file_info->sizeOrg; write_size += buffer_size)
			{
				// Adjust buffer size
				archive->SetBufSize(&buffer_size, write_size);

				// Output
				archive->Read(buffer.data(), buffer_size);
				DecryptPak4(buffer.data(), buffer_size, write_size, file_info);
				archive->WriteFile(buffer.data(), buffer_size);
			}
		}
	}

	return true;
}

bool CNitro::DecodePK2(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".PK2") || memcmp(archive->GetHed(), "ARCV", 4) != 0)
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();
	const YCString file_extension = PathFindExtension(file_info->name);

	if (file_extension != _T(".nps") &&
	    file_extension != _T(".ini") &&
	    file_extension != _T(".h")   &&
	    file_extension != _T(".txt"))
	{
		return false;
	}

	// Get file size
	u32 decompressed_size;
	archive->ReadU32(&decompressed_size);
	decompressed_size &= 0x00FFFFFF;

	// Ensure buffer exists
	std::vector<u8> src(file_info->sizeCmp);
	std::vector<u8> dst(decompressed_size);

	// Reading
	archive->Read(src.data(), src.size());

	// Decompression
	CZlib zlib;
	zlib.Decompress(dst.data(), dst.size(), src.data(), src.size());

	if (file_extension == _T(".nps"))
	{
		// Convert to .txt extension
		archive->OpenScriptFile();
	}
	else
	{
		archive->OpenFile();
	}

	archive->WriteFile(dst.data(), dst.size());

	return true;
}

bool CNitro::DecodeN3Pk(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pak") || memcmp(archive->GetHed(), "N3Pk", 4) != 0)
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();
	const YCString file_extension = PathFindExtension(file_info->name);

	// Ensure buffer exists
	size_t buffer_size = archive->GetBufSize();
	std::vector<u8> buffer(buffer_size);

	u8 byte_key = static_cast<u8>(file_info->key);

	if (file_extension == _T(".nps"))
		archive->OpenScriptFile();
	else
		archive->OpenFile();

	for (size_t write_size = 0; write_size != file_info->sizeOrg; write_size += buffer_size)
	{
		// Ensure buffer exists
		archive->SetBufSize(&buffer_size, write_size);

		// Output
		archive->Read(buffer.data(), buffer_size);
		DecryptN3Pk(buffer.data(), buffer_size, write_size, file_info, byte_key);
		archive->WriteFile(buffer.data(), buffer_size);
	}

	return true;
}

bool CNitro::DecodeNpa(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".npa") || memcmp(archive->GetHed(), "NPA\x01", 4) != 0)
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();
	const YCString file_extension = PathFindExtension(file_info->name);

	if (file_extension == _T(".nss"))
		archive->OpenScriptFile();
	else
		archive->OpenFile();

	if (file_info->format == _T("zlib"))
	{
		// Ensure buffers exist
		std::vector<u8> src(file_info->sizeCmp);
		std::vector<u8> dst(file_info->sizeOrg);

		// Reading
		archive->Read(src.data(), src.size());

		// Decryption
		DecryptNpa(src.data(), src.size(), 0, file_info);

		// Decompression
		CZlib zlib;
		zlib.Decompress(dst.data(), dst.size(), src.data(), src.size());

		// Output
		archive->WriteFile(dst.data(), dst.size());
	}
	else
	{
		// Ensure buffer exists
		size_t buffer_size = archive->GetBufSize();
		std::vector<u8> buffer(buffer_size);

		for (size_t write_size = 0; write_size != file_info->sizeOrg; write_size += buffer_size)
		{
			// Adjust buffer size
			archive->SetBufSize(&buffer_size, write_size);

			// Output
			archive->Read(buffer.data(), buffer_size);
			DecryptNpa(buffer.data(), buffer_size, write_size, file_info);
			archive->WriteFile(buffer.data(), buffer_size);
		}
	}

	return true;
}

void CNitro::DecryptPak3(u8* data, size_t size, size_t offset, const SFileInfo* file_info)
{
	if (file_info->format == _T("zlib"))
	{
		// No decoding
		return;
	}

	// Decoding the first 16 bytes at most
	size = (size < 16) ? size : 16;

	// Decoding
	size_t target_idx = 0;

	for (size_t i = offset; i < size / 4; i++)
	{
		*reinterpret_cast<u32*>(&data[target_idx]) ^= file_info->key;

		target_idx += 4;
	}

	for (size_t i = offset; i < (size & 3); i++)
	{
		data[target_idx++] ^= (file_info->key >> (i * 8)) & 0xFF;
	}
}

void CNitro::DecryptPak4(u8* data, size_t size, size_t offset, const SFileInfo* file_info)
{
	if (file_info->format != _T("zlib"))
	{
		// Decoding up to 1024 bytes from the beginning
		size = (size < 1024) ? size : 1024;
	}

	// Decoding

	size_t target_idx = 0;

	for (size_t i = offset; i < size / 4; i++)
	{
		*reinterpret_cast<u32*>(&data[target_idx]) ^= file_info->key;
		target_idx += 4;
	}

	for (size_t i = offset; i < (size & 3); i++)
	{
		data[target_idx++] ^= (file_info->key >> (i * 8)) & 0xFF;
	}
}

void CNitro::DecryptN3Pk(u8* data, size_t size, size_t offset, const SFileInfo* file_info, u8& byte_key)
{
	static constexpr std::array<u8, 1024> key{{
		0xAA, 0x00, 0x00, 0x00, 0x96, 0x30, 0x07, 0x77, 0x2C, 0x61, 0x0E, 0xEE, 0xBA, 0x51, 0x09, 0x99, 0x19, 0xC4, 0x6D, 0x07, 0x8F, 0xF4, 0x6A, 0x70, 0x35, 0xA5, 0x63, 0xE9, 0xA3, 0x95, 0x64, 0x9E,
		0x32, 0x88, 0xDB, 0x0E, 0xA4, 0xB8, 0xDC, 0x79, 0x1E, 0xE9, 0xD5, 0xE0, 0x88, 0xD9, 0xD2, 0x97, 0x2B, 0x4C, 0xB6, 0x09, 0xBD, 0x7C, 0xB1, 0x7E, 0x07, 0x2D, 0xB8, 0xE7, 0x91, 0x1D, 0xBF, 0x90,
		0x64, 0x10, 0xB7, 0x1D, 0xF2, 0x20, 0xB0, 0x6A, 0x48, 0x71, 0xB9, 0xF3, 0xDE, 0x41, 0xBE, 0x84, 0x7D, 0xD4, 0xDA, 0x1A, 0xEB, 0xE4, 0xDD, 0x6D, 0x51, 0xB5, 0xD4, 0xF4, 0xC7, 0x85, 0xD3, 0x83,
		0x56, 0x98, 0x6C, 0x13, 0xC0, 0xA8, 0x6B, 0x64, 0x7A, 0xF9, 0x62, 0xFD, 0xEC, 0xC9, 0x65, 0x8A, 0x4F, 0x5C, 0x01, 0x14, 0xD9, 0x6C, 0x06, 0x63, 0x63, 0x3D, 0x0F, 0xFA, 0xF5, 0x0D, 0x08, 0x8D,
		0xC8, 0x20, 0x6E, 0x3B, 0x5E, 0x10, 0x69, 0x4C, 0xE4, 0x41, 0x60, 0xD5, 0x72, 0x71, 0x67, 0xA2, 0xD1, 0xE4, 0x03, 0x3C, 0x47, 0xD4, 0x04, 0x4B, 0xFD, 0x85, 0x0D, 0xD2, 0x6B, 0xB5, 0x0A, 0xA5,
		0xFA, 0xA8, 0xB5, 0x35, 0x6C, 0x98, 0xB2, 0x42, 0xD6, 0xC9, 0xBB, 0xDB, 0x40, 0xF9, 0xBC, 0xAC, 0xE3, 0x6C, 0xD8, 0x32, 0x75, 0x5C, 0xDF, 0x45, 0xCF, 0x0D, 0xD6, 0xDC, 0x59, 0x3D, 0xD1, 0xAB,
		0xAC, 0x30, 0xD9, 0x26, 0x3A, 0x00, 0xDE, 0x51, 0x80, 0x51, 0xD7, 0xC8, 0x16, 0x61, 0xD0, 0xBF, 0xB5, 0xF4, 0xB4, 0x21, 0x23, 0xC4, 0xB3, 0x56, 0x99, 0x95, 0xBA, 0xCF, 0x0F, 0xA5, 0xBD, 0xB8,
		0x9E, 0xB8, 0x02, 0x28, 0x08, 0x88, 0x05, 0x5F, 0xB2, 0xD9, 0x0C, 0xC6, 0x24, 0xE9, 0x0B, 0xB1, 0x87, 0x7C, 0x6F, 0x2F, 0x11, 0x4C, 0x68, 0x58, 0xAB, 0x1D, 0x61, 0xC1, 0x3D, 0x2D, 0x66, 0xB6,
		0x90, 0x41, 0xDC, 0x76, 0x06, 0x71, 0xDB, 0x01, 0xBC, 0x20, 0xD2, 0x98, 0x2A, 0x10, 0xD5, 0xEF, 0x89, 0x85, 0xB1, 0x71, 0x1F, 0xB5, 0xB6, 0x06, 0xA5, 0xE4, 0xBF, 0x9F, 0x33, 0xD4, 0xB8, 0xE8,
		0xA2, 0xC9, 0x07, 0x78, 0x34, 0xF9, 0x00, 0x0F, 0x8E, 0xA8, 0x09, 0x96, 0x18, 0x98, 0x0E, 0xE1, 0xBB, 0x0D, 0x6A, 0x7F, 0x2D, 0x3D, 0x6D, 0x08, 0x97, 0x6C, 0x64, 0x91, 0x01, 0x5C, 0x63, 0xE6,
		0xF4, 0x51, 0x6B, 0x6B, 0x62, 0x61, 0x6C, 0x1C, 0xD8, 0x30, 0x65, 0x85, 0x4E, 0x00, 0x62, 0xF2, 0xED, 0x95, 0x06, 0x6C, 0x7B, 0xA5, 0x01, 0x1B, 0xC1, 0xF4, 0x08, 0x82, 0x57, 0xC4, 0x0F, 0xF5,
		0xC6, 0xD9, 0xB0, 0x65, 0x50, 0xE9, 0xB7, 0x12, 0xEA, 0xB8, 0xBE, 0x8B, 0x7C, 0x88, 0xB9, 0xFC, 0xDF, 0x1D, 0xDD, 0x62, 0x49, 0x2D, 0xDA, 0x15, 0xF3, 0x7C, 0xD3, 0x8C, 0x65, 0x4C, 0xD4, 0xFB,
		0x58, 0x61, 0xB2, 0x4D, 0xCE, 0x51, 0xB5, 0x3A, 0x74, 0x00, 0xBC, 0xA3, 0xE2, 0x30, 0xBB, 0xD4, 0x41, 0xA5, 0xDF, 0x4A, 0xD7, 0x95, 0xD8, 0x3D, 0x6D, 0xC4, 0xD1, 0xA4, 0xFB, 0xF4, 0xD6, 0xD3,
		0x6A, 0xE9, 0x69, 0x43, 0xFC, 0xD9, 0x6E, 0x34, 0x46, 0x88, 0x67, 0xAD, 0xD0, 0xB8, 0x60, 0xDA, 0x73, 0x2D, 0x04, 0x44, 0xE5, 0x1D, 0x03, 0x33, 0x5F, 0x4C, 0x0A, 0xAA, 0xC9, 0x7C, 0x0D, 0xDD,
		0x3C, 0x71, 0x05, 0x50, 0xAA, 0x41, 0x02, 0x27, 0x10, 0x10, 0x0B, 0xBE, 0x86, 0x20, 0x0C, 0xC9, 0x25, 0xB5, 0x68, 0x57, 0xB3, 0x85, 0x6F, 0x20, 0x09, 0xD4, 0x66, 0xB9, 0x9F, 0xE4, 0x61, 0xCE,
		0x0E, 0xF9, 0xDE, 0x5E, 0x98, 0xC9, 0xD9, 0x29, 0x22, 0x98, 0xD0, 0xB0, 0xB4, 0xA8, 0xD7, 0xC7, 0x17, 0x3D, 0xB3, 0x59, 0x81, 0x0D, 0xB4, 0x2E, 0x3B, 0x5C, 0xBD, 0xB7, 0xAD, 0x6C, 0xBA, 0xC0,
		0x20, 0x83, 0xB8, 0xED, 0xB6, 0xB3, 0xBF, 0x9A, 0x0C, 0xE2, 0xB6, 0x03, 0x9A, 0xD2, 0xB1, 0x74, 0x39, 0x47, 0xD5, 0xEA, 0xAF, 0x77, 0xD2, 0x9D, 0x15, 0x26, 0xDB, 0x04, 0x83, 0x16, 0xDC, 0x73,
		0x12, 0x0B, 0x63, 0xE3, 0x84, 0x3B, 0x64, 0x94, 0x3E, 0x6A, 0x6D, 0x0D, 0xA8, 0x5A, 0x6A, 0x7A, 0x0B, 0xCF, 0x0E, 0xE4, 0x9D, 0xFF, 0x09, 0x93, 0x27, 0xAE, 0x00, 0x0A, 0xB1, 0x9E, 0x07, 0x7D,
		0x44, 0x93, 0x0F, 0xF0, 0xD2, 0xA3, 0x08, 0x87, 0x68, 0xF2, 0x01, 0x1E, 0xFE, 0xC2, 0x06, 0x69, 0x5D, 0x57, 0x62, 0xF7, 0xCB, 0x67, 0x65, 0x80, 0x71, 0x36, 0x6C, 0x19, 0xE7, 0x06, 0x6B, 0x6E,
		0x76, 0x1B, 0xD4, 0xFE, 0xE0, 0x2B, 0xD3, 0x89, 0x5A, 0x7A, 0xDA, 0x10, 0xCC, 0x4A, 0xDD, 0x67, 0x6F, 0xDF, 0xB9, 0xF9, 0xF9, 0xEF, 0xBE, 0x8E, 0x43, 0xBE, 0xB7, 0x17, 0xD5, 0x8E, 0xB0, 0x60,
		0xE8, 0xA3, 0xD6, 0xD6, 0x7E, 0x93, 0xD1, 0xA1, 0xC4, 0xC2, 0xD8, 0x38, 0x52, 0xF2, 0xDF, 0x4F, 0xF1, 0x67, 0xBB, 0xD1, 0x67, 0x57, 0xBC, 0xA6, 0xDD, 0x06, 0xB5, 0x3F, 0x4B, 0x36, 0xB2, 0x48,
		0xDA, 0x2B, 0x0D, 0xD8, 0x4C, 0x1B, 0x0A, 0xAF, 0xF6, 0x4A, 0x03, 0x36, 0x60, 0x7A, 0x04, 0x41, 0xC3, 0xEF, 0x60, 0xDF, 0x55, 0xDF, 0x67, 0xA8, 0xEF, 0x8E, 0x6E, 0x31, 0x79, 0xBE, 0x69, 0x46,
		0x8C, 0xB3, 0x61, 0xCB, 0x1A, 0x83, 0x66, 0xBC, 0xA0, 0xD2, 0x6F, 0x25, 0x36, 0xE2, 0x68, 0x52, 0x95, 0x77, 0x0C, 0xCC, 0x03, 0x47, 0x0B, 0xBB, 0xB9, 0x16, 0x02, 0x22, 0x2F, 0x26, 0x05, 0x55,
		0xBE, 0x3B, 0xBA, 0xC5, 0x28, 0x0B, 0xBD, 0xB2, 0x92, 0x5A, 0xB4, 0x2B, 0x04, 0x6A, 0xB3, 0x5C, 0xA7, 0xFF, 0xD7, 0xC2, 0x31, 0xCF, 0xD0, 0xB5, 0x8B, 0x9E, 0xD9, 0x2C, 0x1D, 0xAE, 0xDE, 0x5B,
		0xB0, 0xC2, 0x64, 0x9B, 0x26, 0xF2, 0x63, 0xEC, 0x9C, 0xA3, 0x6A, 0x75, 0x0A, 0x93, 0x6D, 0x02, 0xA9, 0x06, 0x09, 0x9C, 0x3F, 0x36, 0x0E, 0xEB, 0x85, 0x67, 0x07, 0x72, 0x13, 0x57, 0x00, 0x05,
		0x82, 0x4A, 0xBF, 0x95, 0x14, 0x7A, 0xB8, 0xE2, 0xAE, 0x2B, 0xB1, 0x7B, 0x38, 0x1B, 0xB6, 0x0C, 0x9B, 0x8E, 0xD2, 0x92, 0x0D, 0xBE, 0xD5, 0xE5, 0xB7, 0xEF, 0xDC, 0x7C, 0x21, 0xDF, 0xDB, 0x0B,
		0xD4, 0xD2, 0xD3, 0x86, 0x42, 0xE2, 0xD4, 0xF1, 0xF8, 0xB3, 0xDD, 0x68, 0x6E, 0x83, 0xDA, 0x1F, 0xCD, 0x16, 0xBE, 0x81, 0x5B, 0x26, 0xB9, 0xF6, 0xE1, 0x77, 0xB0, 0x6F, 0x77, 0x47, 0xB7, 0x18,
		0xE6, 0x5A, 0x08, 0x88, 0x70, 0x6A, 0x0F, 0xFF, 0xCA, 0x3B, 0x06, 0x66, 0x5C, 0x0B, 0x01, 0x11, 0xFF, 0x9E, 0x65, 0x8F, 0x69, 0xAE, 0x62, 0xF8, 0xD3, 0xFF, 0x6B, 0x61, 0x45, 0xCF, 0x6C, 0x16,
		0x78, 0xE2, 0x0A, 0xA0, 0xEE, 0xD2, 0x0D, 0xD7, 0x54, 0x83, 0x04, 0x4E, 0xC2, 0xB3, 0x03, 0x39, 0x61, 0x26, 0x67, 0xA7, 0xF7, 0x16, 0x60, 0xD0, 0x4D, 0x47, 0x69, 0x49, 0xDB, 0x77, 0x6E, 0x3E,
		0x4A, 0x6A, 0xD1, 0xAE, 0xDC, 0x5A, 0xD6, 0xD9, 0x66, 0x0B, 0xDF, 0x40, 0xF0, 0x3B, 0xD8, 0x37, 0x53, 0xAE, 0xBC, 0xA9, 0xC5, 0x9E, 0xBB, 0xDE, 0x7F, 0xCF, 0xB2, 0x47, 0xE9, 0xFF, 0xB5, 0x30,
		0x1C, 0xF2, 0xBD, 0xBD, 0x8A, 0xC2, 0xBA, 0xCA, 0x30, 0x93, 0xB3, 0x53, 0xA6, 0xA3, 0xB4, 0x24, 0x05, 0x36, 0xD0, 0xBA, 0x93, 0x06, 0xD7, 0xCD, 0x29, 0x57, 0xDE, 0x54, 0xBF, 0x67, 0xD9, 0x23,
		0x2E, 0x7A, 0x66, 0xB3, 0xB8, 0x4A, 0x61, 0xC4, 0x02, 0x1B, 0x68, 0x5D, 0x94, 0x2B, 0x6F, 0x2A, 0x37, 0xBE, 0x0B, 0xB4, 0xA1, 0x8E, 0x0C, 0xC3, 0x1B, 0xDF, 0x05, 0x5A, 0x8D, 0xEF, 0x02, 0x2D
	}};

	switch (file_info->type)
	{
	case 1: // Decode all sizes
		for (size_t i = 0; i < size; i++)
		{
			data[i] ^= key[byte_key++];
		}
		break;

	case 2: // Decoding up to 1024 bytes from the beginning
		size = (size < 1024) ? size : 1024;

		for (size_t i = offset, j = 0; i < size; i++, j++)
		{
			data[j] ^= key[byte_key++];
		}
		break;
	}
}

void CNitro::DecryptNpa(u8* data, size_t size, size_t offset, const SFileInfo* file_info)
{
	static constexpr std::array<u8, 256> key{{
		0xDF, 0x5F, 0x6E, 0xF7, 0xF5, 0xEF, 0x52, 0x5B, 0x7E, 0x25, 0xD7, 0x46, 0xBC, 0x92, 0x02, 0x2E, 0x51, 0x7C, 0x39, 0x16, 0x2A, 0x18, 0x08, 0xEB, 0x0C, 0x97, 0x3A, 0xC7, 0xAC, 0xC6, 0xB0, 0x17,
		0x80, 0xD6, 0x86, 0x3C, 0xFB, 0xF9, 0xB1, 0x01, 0xA9, 0x79, 0x9E, 0xB3, 0x37, 0xDE, 0x19, 0xE7, 0x2B, 0xC2, 0x28, 0x1E, 0x5D, 0x67, 0x22, 0x8E, 0x58, 0x1A, 0xCC, 0xEC, 0x44, 0x9D, 0xA7, 0x24,
		0x55, 0x0F, 0x64, 0x56, 0x4C, 0x6C, 0xC4, 0x77, 0x11, 0x09, 0xEA, 0xC8, 0x20, 0x63, 0x1C, 0x6D, 0x3D, 0x72, 0xB8, 0x8A, 0x54, 0x95, 0x3F, 0x98, 0xE9, 0xB2, 0x7F, 0x06, 0x50, 0x8C, 0xC5, 0xB4,
		0x9B, 0x31, 0x81, 0x2D, 0x7B, 0xA3, 0x42, 0x3B, 0xF6, 0x74, 0xE0, 0x83, 0xA0, 0x41, 0xD2, 0xEE, 0x49, 0xC9, 0x57, 0x73, 0x61, 0x85, 0x6A, 0x78, 0x89, 0x68, 0x2C, 0xB5, 0xB9, 0xA8, 0x40, 0x05,
		0xE8, 0xBF, 0xBB, 0x26, 0xA4, 0x66, 0x94, 0x87, 0x2F, 0xFE, 0xDC, 0x33, 0xCE, 0xB7, 0xE4, 0x07, 0x43, 0x48, 0xCB, 0xC3, 0x76, 0x30, 0xDD, 0x6B, 0x21, 0xD4, 0x82, 0xAD, 0x84, 0xD0, 0x75, 0xF1,
		0xF0, 0xE2, 0xDB, 0x03, 0x6F, 0x65, 0xCD, 0x0D, 0x4A, 0x62, 0x0A, 0x88, 0x8F, 0x8D, 0x14, 0xAB, 0x70, 0xED, 0x0B, 0x45, 0x23, 0xC0, 0xAA, 0x91, 0x0E, 0x3E, 0x29, 0xD1, 0x59, 0xFD, 0xFA, 0xE1,
		0xA2, 0xAF, 0xF4, 0x4F, 0x4D, 0xA5, 0x9C, 0xE3, 0x8B, 0x00, 0x12, 0x90, 0x32, 0x15, 0xD5, 0xBE, 0xD8, 0x4E, 0x69, 0xF2, 0xE6, 0x9A, 0xCA, 0x99, 0x04, 0x7A, 0x35, 0x10, 0x38, 0x1D, 0x13, 0x4B,
		0xA6, 0xA1, 0x47, 0x5A, 0x1F, 0x36, 0xC1, 0x53, 0xB6, 0xCF, 0xAE, 0x7D, 0xFF, 0x93, 0x71, 0x34, 0xD3, 0xFC, 0x9F, 0xF8, 0x5E, 0x1B, 0xD9, 0x60, 0xBA, 0xBD, 0x5C, 0xE5, 0xF3, 0x27, 0xDA, 0x96
	}};

	const u8 file_key = static_cast<u8>(file_info->key & 0xFF);

	// Decoding up to 4096 bytes from the beginning
	size = (size < 4096) ? size : 4096;

	// Decoding
	for (size_t i = offset, j = 0; i < size; i++, j++)
	{
		data[j] = key[data[j]] - file_key;
	}
}
