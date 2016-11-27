#include "StdAfx.h"
#include "Extract/Axl.h"

#include "ArcFile.h"
#include "Image.h"
#include "Arc/LZSS.h"

bool CAxl::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".arc"))
		return false;

	if (*(u32*)&archive->GetHeader()[4] != 0 && *(u32*)&archive->GetHeader()[4] != 1)
		return false;

	// Get file count
	const u32 num_files = *(u32*)&archive->GetHeader()[0];

	// Get filetype
	const u32 type = *(u32*)&archive->GetHeader()[4];

	// Number of files retrieved from the index size
	const u32 index_size = num_files * 44;

	// Archive file size cannot be smaller than the index size
	if (archive->GetArcSize() < index_size)
		return false;

	archive->Seek(8, FILE_BEGIN);

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	if (!DecryptIndex(index.data(), index.size(), archive->GetArcSize()))
	{
		// Cannot be decoded
		archive->SeekHed();
		return false;
	}

	const u8* index_ptr = index.data();
	for (u32 i = 0; i < num_files; i++)
	{
		// Get filename
		TCHAR file_name[33];
		memcpy(file_name, index_ptr, 32);
		file_name[32] = _T('\0');

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.sizeOrg = *(const u32*)&index_ptr[32];
		file_info.sizeCmp = *(const u32*)&index_ptr[36];
		file_info.start = *(const u32*)&index_ptr[40];
		file_info.end = file_info.start + file_info.sizeCmp;
		if (type == 1)
			file_info.format = _T("LZ");
		file_info.title = _T("AXL");
		archive->AddFileInfo(file_info);

		index_ptr += 44;
	}

	return true;
}

bool CAxl::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->title != _T("AXL"))
		return false;

	// Ensure buffer
	std::vector<u8> buf(file_info->sizeOrg);

	// LZ Compressed File
	if (file_info->format == _T("LZ"))
	{
		// Reading
		std::vector<u8> z_buf(file_info->sizeCmp);
		archive->Read(z_buf.data(), z_buf.size());

		// LZSS Decompression
		CLZSS lzss;
		lzss.Decomp(buf.data(), buf.size(), z_buf.data(), z_buf.size(), 4096, 4078, 3);
	}
	else // Uncompressed File
	{
		archive->Read(buf.data(), file_info->sizeCmp);
	}

	// BMP File
	if (lstrcmpi(PathFindExtension(file_info->name), _T(".bmp")) == 0)
	{
		const auto* const file_header = (const BITMAPFILEHEADER*)&buf[0];
		const auto* const info_header = (const BITMAPINFOHEADER*)&buf[14];

		if (file_header->bfSize != file_info->sizeOrg)
		{
			// 32bit BMP

			// Make buffer for 32bit BMP
			std::vector<u8> buf2(file_info->sizeOrg - 54);

			// Refers to the alpha value
			const u8* pbufA = &buf[file_header->bfSize];

			// Skip the BMP header
			const u8* pbuf = &buf[54];

			u8* pbuf2 = &buf2[0];

			const size_t color_size = info_header->biWidth * info_header->biHeight;

			for (size_t i = 0; i < color_size; i++)
			{
				// Synthesize alpha value
				for (size_t j = 0; j < 3; j++)
				{
					*pbuf2++ = *pbuf++;
				}

				*pbuf2++ = *pbufA++;
			}

			CImage image;
			image.Init(archive, info_header->biWidth, info_header->biHeight, 32);
			image.Write(&buf2[0], file_info->sizeOrg - 54);
		}
		else // Below 24bit BMP
		{
			CImage image;
			image.Init(archive, buf.data());
			image.Write(buf.size());
		}
	}
	else if (file_info->format == _T("LZ"))
	{
		// LZ Compressed files other than BMP
		archive->OpenFile();
		archive->WriteFile(buf.data(), buf.size());
	}
	else // Other file
	{
		archive->OpenFile();
		archive->WriteFile(buf.data(), file_info->sizeCmp);
	}

	return true;
}

void CAxl::InitMountKey(const void* decryption_key)
{
	m_length = strlen(static_cast<const char*>(decryption_key));
	memcpy(m_decryption_key, decryption_key, m_length);
}

bool CAxl::CreateKey(u8* key, size_t* key_length, const u8* index, size_t index_size)
{
	for (size_t i = 0; i < index_size; i += 44)
	{
		// Copy the portion of the file name from the index
		u8 fname[32];
		memcpy(fname, index, 32);

		const u8* key_end = &fname[31];

		const u8* pkey1 = key_end;
		const u8* pkey2 = key_end - 1;

		*key_length = 1;

		while (pkey2 > fname)
		{
			while (*pkey1 != *pkey2)
			{
				// Moved if the same data
				pkey2--;

				// Keep counting the length of the key
				(*key_length)++;

				// After exiting the loop, go back to the beginning of the file name
				if (pkey2 == fname)
					break;
			}

			// After exiting the loop, go back to the beginning of the file name
			if (pkey2 == fname)
				break;

			size_t key_len_cpy = *key_length;

			// Make a comparison with previous data
			while (*pkey1 == *pkey2)
			{
				pkey1--;
				pkey2--;
				key_len_cpy--;

				// Once the key matches exactly
				if (key_len_cpy == 0)
				{
					// Length of the key to be copied to the first
					const size_t key_len_hed = (i + 32) % *key_length;
					const u8* pkey3 = key_end + 1 - key_len_hed;

					for (size_t j = 0; j < key_len_hed; j++)
						key[j] = *pkey3++;

					// Followed by the actual key
					pkey3 = key_end + 1 - *key_length;

					for (size_t j = key_len_hed; j < *key_length; j++)
						key[j] = *pkey3++;

					return true;
				}

				// After exiting the loop, go back to the beginning of the file name
				if (pkey2 == fname)
					break;
			}

			// After exiting the loop, go back to the beginning of the file name
			if (pkey2 == fname)
				break;
		}

		index += 44;
	}

	return false;
}

bool CAxl::DecryptIndex(u8* index, size_t index_size, u64 archive_size)
{
	// Key generation from the file name portion of the index
	u8 key[32];
	size_t key_len;
	if (!CreateKey(key, &key_len, index, index_size))
		return false;

	// Copy the index
	std::vector<u8> index_copy(index_size);
	memcpy(index_copy.data(), index, index_size);

	// Decoding the index that was copied
	for (size_t i = 0, j = 0; i < index_size; i++)
	{
		index_copy[i] -= key[j++];

		if (j == key_len)
		{
			j = 0;
		}
	}

	// Check whether the index matches the beginning and end of the first file
	if (*(u32*)&index_copy[40] != 8 + index_size)
		return false;

	// Check whether the match is the end of the last file and archive files
	if (*(u32*)&index_copy[index_size - 4] + *(u32*)&index_copy[index_size - 8] != archive_size)
		return false;

	// The copied index was the result of the check if there is no problem decoding
	memcpy(index, index_copy.data(), index_size);

	return true;

/*
	static constexpr std::array<const char*, 11> key{{
		"SUMMER",     // チュートリアルサマー
		"HIDAMARI",   // ひだまり
		"KIMIKOE",    // キミの声がきこえる
		"CFTRIAL",    // CROSS FIRE Trial Version v1
		"TEST",       // CROSS FIRE, CROSS FIRE Trial Version v2
		"SAKURA-R",   // さくらリラクゼーション
		"KOICHU",     // こいちゅ！ ～恋に恋するかたおもい～
		"LAVENDURA",  // 黎明のラヴェンデュラ
		"HAMA",       // 真章 幻夢館, そらのいろ、みずのいろ
		"OBA",        // 叔母の寝室
		"KANSEN"      // 姦染 ～淫欲の連鎖～
	}};

	std::vector<u8> index_copy(index_size);

	for (size_t i = 0; i < key.size(); i++)
	{
		// Copy the index
		memcpy(index_copy.data(), index, index_size);

		// Determine length of the key
		const size_t key_len = strlen((char*)key[i]);

		// Decryption
		for (size_t j = 0, k = 0; j < index_size; j++)
		{
			index_copy[j] += static_cast<u8>(key[i][k++]);
			if (k == key_len)
				k = 0;
		}

		// Check for a match between the beginning and end of the index of the first file
		// Decrypt another key if they do not match
		if (*(u32*)&index_copy[40] != 8 + index_size)
			continue;

		// Check whether the match is the end of the last file and the end of the archive files
		// If there is a match
		if (*(u32*)&index_copy[index_size - 4] + *(u32*)&index_copy[index_size - 8] == archive_size)
		{
			memcpy(index, index_copy.data(), index_size);
			return true;
		}
	}

	return false;
*/


	//const u8* deckey = m_deckey;
	//size_t deckey_len = m_len;

	//for (size_t i = 0, j = 0; i < index_size; i++)
	//{
	//	index[i] += deckey[j++];
	//	if (j == deckey_len)
	//      j = 0;
	//}
}
