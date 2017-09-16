#include "StdAfx.h"
#include "QLIE.h"

#include "ArcFile.h"
#include "Image.h"

bool CQLIE::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pack"))
		return false;

	// Get the signature
	u8 signature[16];
	archive->Seek(-28, FILE_END);
	archive->Read(signature, sizeof(signature));

	// Check the signature
	if (memcmp(signature, "FilePackVer", 11) != 0)
	{
		archive->SeekHed();
		return false;
	}

	// Get the file count
	u32 num_files;
	archive->ReadU32(&num_files);

	// Get the index position/offset
	u64 index_ofs;
	archive->ReadU64(&index_ofs);

	// Go to the index position
	archive->Seek(index_ofs, FILE_BEGIN);

	// Get the index size (Ver2.0 or later will be included in the hash size)
	const u32 index_size = archive->GetArcSize() - 28 - index_ofs;

	// Read the index (Ver2.0 or later will be included in the hash)
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	u32       seed = 0;
	const u8* pIndex = index.data();
	YCString  version;
	void (*DecryptFunc)(u8*, u32, u32);

	// Check the version of 'pack'
	if (memcmp(signature, "FilePackVer1.0", 14) == 0 || memcmp(signature, "FilePackVer2.0", 14) == 0)
	{
		// Version
		version = _T("FilePackVer1.0");

		// Set the decoding functions
		DecryptFunc = DecryptFileName;
	}
	else if (memcmp(signature, "FilePackVer3.0", 14) == 0)
	{
		const u8* pIndex2 = pIndex;

		// Figure out how much data is TOC entries..
		for (int i = 0; i < (int)num_files; i++)
		{
			const u16 filename_len = *(const u16*)pIndex2;
			pIndex2 += 2 + filename_len + 28;
		}

		// Compute the obfuscation seed from the CRC(?) of some hash data.
		const u8* hash_bytes = pIndex2 + 32 + *(const u32*)&pIndex2[28] + 36;
		seed = crc_or_something(hash_bytes, 256) & 0x0FFFFFFF;

		// Version
		version = _T("FilePackVer3.0");

		// Set the decoding functions
		DecryptFunc = DecryptFileNameV3;
	}
	else
	{
		archive->SeekHed();
		return false;
	}

	for (u32 i = 0; i < num_files; i++)
	{
		// Get the filename length
		const u16 file_name_len = *(const u16*)&pIndex[0];
		
		//archive->Read(&FileNameLen, 2);
		u8 file_name[_MAX_FNAME];
		memcpy(file_name, &pIndex[2], file_name_len);
		
		//archive->Read(szFileName, FileNameLen);
    file_name[file_name_len] = '\0';
		
		// Get the filename
		DecryptFunc(file_name, file_name_len, seed);

		pIndex += 2 + file_name_len;

		//u8 index[28];
		//archive->Read(&index, 28);

		// Add to the listview.
		SFileInfo info;
		info.name = (char*)file_name;
		info.start = *(const u64*)&pIndex[0];
		info.sizeCmp = *(const u32*)&pIndex[8];
		info.sizeOrg = *(const u32*)&pIndex[12];
		info.end = info.start + info.sizeCmp;
		info.key = (version == _T("FilePackVer1.0")) ? *(const u32*)&pIndex[20] : seed;
		info.title = version;
		archive->AddFileInfo(info);

		pIndex += 28;
	}

	return true;
}

bool CQLIE::Decode(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pack"))
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Ensure buffers exist
	std::vector<u8> z_buf(file_info->sizeCmp);
	std::vector<u8> buf; // Only assigned when the file is uncompressed and when resize is called.(Memory saving)

	// Read the file
	archive->Read(z_buf.data(), z_buf.size());

	// Decrypt the encrypted file
	if (file_info->title == _T("FilePackVer1.0"))
	{
		if (file_info->key)
			Decrypt(z_buf.data(), z_buf.size(), 0);
	}
	else if (file_info->title == _T("FilePackVer3.0"))
	{
		DecryptV3(z_buf.data(), z_buf.size(), file_info->key);
	}

	u8* pBuf = z_buf.data();

	// Extract the file if it's compressed
	if (file_info->sizeCmp != file_info->sizeOrg)
	{
		buf.resize(file_info->sizeOrg);
		Decomp(buf.data(), buf.size(), z_buf.data(), z_buf.size());
		pBuf = buf.data();
	}

	// Output
	if (file_info->format == _T("BMP"))
	{
		const LPBITMAPFILEHEADER file = (LPBITMAPFILEHEADER)&pBuf[0];
		const LPBITMAPINFOHEADER info = (LPBITMAPINFOHEADER)&pBuf[14];

		// Output size
		u32 dst_size = file_info->sizeOrg - 54;

		if (((info->biWidth * (info->biBitCount >> 3) + 3) & 0xFFFFFFFC) * info->biHeight != dst_size)
			dst_size -= 2;

		// Output
		CImage image;
		image.Init(archive, info->biWidth, info->biHeight, info->biBitCount, &pBuf[54], file->bfOffBits - 54);
		image.Write(&pBuf[file->bfOffBits], dst_size);
	}
	else if (file_info->format == _T("B"))
	{
		// *.b file
		if (!DecodeB(archive, pBuf, file_info->sizeOrg))
		{
			// Unsupported
			archive->OpenFile();
			archive->WriteFile(pBuf, file_info->sizeOrg);
		}
	}
	else
	{
		archive->OpenFile();
		archive->WriteFile(pBuf, file_info->sizeOrg);
	}

	return true;
}

/// *.b file decompressing
///
/// @param archive  Archive
/// @param src      .b file
/// @param src_size .b file size
///
bool CQLIE::DecodeB(CArcFile* archive, u8* src, u32 src_size)
{
	if (memcmp(&src[0], "ABMP7", 5) == 0)
	{
		// ABMP7
		return DecodeABMP7(archive, src, src_size);
	}

	if (memcmp(&src[0], "abmp", 4) == 0)
	{
		// abmp10
		return DecodeABMP10(archive, src, src_size);
	}

	// Not supported
	return false;
}

/// ABMP7 Decompressing
///
/// @param archive       Archive
/// @param src           .b file
/// @param src_size      .b file size
/// @param src_index_ptr Entire .b file index (You'll need to add the minutes after the recursive call)
/// @param b_file_name   Current .b file name (Will change with each recursive call)
///
bool CQLIE::DecodeABMP7(CArcFile* archive, u8* src, u32 src_size, u32* src_index_ptr, const YCString& b_file_name)
{
	u32 src_index = 0;

	if ((src_index + 16) > src_size)
	{
		// Exit
		return false;
	}

	if (memcmp(&src[src_index], "ABMP7", 5) != 0)
	{
		// Unsupported
		return false;
	}

	src_index += 12;

	// Get the amount of unnecessary data we can skip
	const u32 unknown_data_size = *(u32*)&src[src_index];
	src_index += 4;

	// Skip the unnecessary data
	src_index += unknown_data_size;

	//-- First read ------------------------------------------------------------------------

	// Get filesize
	u32 file_size = *(u32*)&src[src_index];
	src_index += 4;

	// Get the file extension
	YCString file_extension = GetExtension(&src[src_index]);

	// Output
	if (file_extension == _T(".bmp"))
	{
		// BITMAP
		CImage image;
		image.Init(archive, &src[src_index]);
		image.Write(file_size);
		image.Close();
	}
	else
	{
		// Other
		archive->OpenFile(file_extension);
		archive->WriteFile(&src[src_index], file_size);
		archive->CloseFile();
	}

	src_index += file_size;

	//-- Second read (and subsequent reads) -----------------------------------------------------------------

	while (src_index < src_size)
	{
		// Get the length of the filename
		const u32 file_name_length = src[src_index];

		src_index += 1;

		// Get the filename
		char file_name[_MAX_FNAME];
		memcpy(file_name, &src[src_index], file_name_length);
		file_name[file_name_length] = '\0';

		src_index += 31;

		// Get the file size
		file_size = *(u32*)&src[src_index];
		src_index += 4;

		if (file_size == 0)
		{
			continue;
		}

		// Get file extension
		file_extension = GetExtension(&src[src_index]);

		// Write the .b filename extension
		TCHAR work[_MAX_FNAME];
		_stprintf(work, _T("_%s%s"), file_name, file_extension.GetString());

		// Output
		if (file_extension == _T(".bmp"))
		{
			// BITMAP
			CImage image;
			image.Init(archive, &src[src_index], work);
			image.Write(file_size);
			image.Close();
		}
		else
		{
			// Other
			archive->OpenFile(work);
			archive->WriteFile(&src[src_index], file_size);
			archive->CloseFile();
		}

		src_index += file_size;
	}

	return true;
}

/// abmp10~12 Decompression
///
/// @param archive       Archive
/// @param src           .b file
/// @param src_size      .b file size
/// @param src_index_ptr Entire .b file index (You'll need to add the minutes after the recursive call)
/// @param b_file_name   Current .b file name (Will change with each recursive call)
///
bool CQLIE::DecodeABMP10(CArcFile* archive, u8* src, u32 src_size, u32* src_index_ptr, const YCString& b_file_name)
{
	static u32 dst_files;
	static std::vector<FileNameInfo> file_names;

	if (src_index_ptr == nullptr)
	{
		// First call
		dst_files = 0;
		file_names.clear();
	}

	u32 src_index = 0;

	if ((src_index + 16) > src_size)
	{
		// Exit
		return false;
	}

	if (memcmp(&src[src_index], "abmp", 4) != 0)
	{
		// Unsupported
		return false;
	}

	src_index += 16;

	if (memcmp(&src[src_index], "abdata", 6) != 0)
	{
		// Unsupported
		return false;
	}

	src_index += 16;

	// Get the amount of data we can skip
	const u32 unknown_data_size = *(u32*)&src[src_index];
	src_index += 4;

	// Skip the unnecessary data
	src_index += unknown_data_size;

	while (src_index < src_size)
	{
		if ((src_index + 16) > src_size)
		{
			// Exit
			break;
		}

		if (memcmp(&src[src_index], "abimage", 7) == 0)
		{
			// abimage10
		}
		else if (memcmp(&src[src_index], "absound", 7) == 0)
		{
			// absound10
		}
		else
		{
			// Unsupported
			return false;
		}

		src_index += 16;

		// Get file count
		const u32 num_files = src[src_index];
		src_index += 1;

		for (u32 i = 0; i < num_files; i++)
		{
			if ((src_index + 16) > src_size)
			{
				// Exit
				break;
			}

			u32 dat_version;

			if (memcmp(&src[src_index], "abimgdat10", 10) == 0)
			{
				// abimgdat10
				dat_version = ABIMGDAT10;
			}
			else if (memcmp(&src[src_index], "abimgdat11", 10) == 0)
			{
				// abimgdat11
				dat_version = ABIMGDAT11;
			}
			else if (memcmp(&src[src_index], "absnddat10", 10) == 0)
			{
				// absnddat10
				dat_version = ABSNDDAT10;
			}
			else if (memcmp(&src[src_index], "absnddat11", 10) == 0)
			{
				// absnddat11
				dat_version = ABSNDDAT11;
			}
			else
			{
				// Unsupported
				return false;
			}

			src_index += 16;

			// Get the length of the filename
			const u32 file_name_length = *(u16*)&src[src_index];
			src_index += 2;

			// Get the filename
			YCString file_name((char*)&src[src_index], file_name_length);
			src_index += file_name_length;

			// Skip unknown data
			if (dat_version == ABIMGDAT11 || dat_version == ABSNDDAT11)
			{
				const u16 length = *(u16*)&src[src_index];

				src_index += 2 + length;
			}

			src_index += 1;

			// Get file size
			const u32 file_size = *(u32*)&src[src_index];
			src_index += 4;

			if (file_size == 0)
			{
				// File size is 0
				continue;
			}

			// Get the file extension
			const YCString file_extension = GetExtension(&src[src_index]);

			if (file_extension == _T(".b"))
			{
				// abmp10
				YCString abmp10_file_name;

				if (file_name == _T(""))
				{
					abmp10_file_name = b_file_name;
				}
				else
				{
					abmp10_file_name = b_file_name + _T("_") + file_name;
				}

				DecodeABMP10(archive, &src[src_index], file_size, &src_index, abmp10_file_name);

				continue;
			}

			// Replace '/' with '_' (Taking into account double-byte characters)
			file_name.Replace(_T('/'), _T('_'));

			// Erase illegal chars
			EraseNotUsePathWord(file_name);

			// We rename the extension as there may be a psd file with the same extension
			file_name.RenameExtension(file_extension);

			// Write the .b extension
			TCHAR work[_MAX_FNAME];

			if (file_name_length == 0)
			{
				// No filename (only put extension)
				_stprintf(work, _T("%s%s"), b_file_name.GetString(), file_name.GetString());
			}
			else
			{
				_stprintf(work, _T("%s_%s"), b_file_name.GetString(), file_name.GetString());
			}

			// Check to see if the same filename doesn't exist
			for (size_t index = 0; index < file_names.size(); index++)
			{
				if (file_names[index].file_name == work)
				{
					// Same filename
					file_names[index].count++;

					// Overwrite will not occur
					TCHAR renamed_extension[256];
					_stprintf(renamed_extension, _T("_%d%s"), file_names[index].count, file_name.GetFileExt().GetString());

					PathRenameExtension(work, renamed_extension);

					// Reset the loop counter to ensure the same file does not exist now.
					index = (size_t)-1;
				}
			}

			// Register the filename
			FileNameInfo file_name_info;
			file_name_info.file_name = work;
			file_name_info.count = 1;

			file_names.push_back(file_name_info);

			// Output
			if (file_extension == _T(".bmp"))
			{
				// BITMAP
				CImage image;
				image.Init(archive, &src[src_index], work);
				image.Write(file_size);
				image.Close();
			}
			else
			{
				// Other
				archive->OpenFile(work);
				archive->WriteFile(&src[src_index], file_size);
				archive->CloseFile();
			}

			dst_files++;

			src_index += file_size;
		}
	}

	if (src_index_ptr != nullptr)
	{
		// Recursive call being performed
		*src_index_ptr += src_index;
	}
	else
	{
		if (dst_files == 0)
		{
			// No output files
			return false;
		}
	}

	return true;
}

/// Getting the file extension
///
/// @param src Input buffer
///
YCString CQLIE::GetExtension(const u8* src)
{
	YCString file_extension;

	if (memcmp(&src[0], "\x89PNG", 4) == 0)
	{
		// PNG
		file_extension = _T(".png");
	}
	else if (memcmp(&src[0], "\xFF\xD8\xFF\xE0", 4) == 0)
	{
		// JPEG
		file_extension = _T(".jpg");
	}
	else if (memcmp(&src[0], "BM", 2) == 0)
	{
		// BITMAP
		file_extension = _T(".bmp");
	}
	else if (memcmp(&src[0], "OggS", 4) == 0)
	{
		// Ogg Vorbis
		file_extension = _T(".ogg");
	}
	else if (memcmp(&src[0], "RIFF", 4) == 0)
	{
		// WAVE
		file_extension = _T(".wav");
	}
	else if (memcmp(&src[0], "abmp", 4) == 0)
	{
		// abmp10
		file_extension = _T(".b");
	}

	return file_extension;
}

/// Erase path characters
///
/// @param path Path
///
void CQLIE::EraseNotUsePathWord(YCString& path)
{
	static const TCHAR unusable_path_characters[] =
	{
		_T(':'), _T(','), _T(';'), _T('*'), _T('?'), _T('\"'), _T('<'), _T('>'), _T('|')
	};

	for (const TCHAR& c : unusable_path_characters)
	{
		path.Remove(c);
	}
}


// DecryptFileName, Decrypt, Decompは、
// 十一寒月氏が作成・公開しているgageのソースコードを参考にして作成しました。

void CQLIE::DecryptFileName(u8* in, u32 size, u32 seed)
{
	for (int i = 1; i <= (int)size; i++)
	{
		*in++ ^= (i + (i ^ ((0xFA + size) & 0xFF))) & 0xFF;
	}
}

void CQLIE::Decrypt(u8* buf, u32 buf_len, u32 seed)
{
	u32 key1 = buf_len ^ 0xFEC9753E;
	u32 key2 = key1;
	u32 x1 = 0xA73C5F9D;
	u32 x2 = x1;
	const u32 size = buf_len >> 3;

	for (size_t i = 0; i < size; i++)
	{
		x1 = (x1 + 0xCE24F523) ^ key1;
		x2 = (x2 + 0xCE24F523) ^ key2;
		key1 = ((u32*)buf)[i*2+0] ^= x1;
		key2 = ((u32*)buf)[i*2+1] ^= x2;
	}
}

void CQLIE::Decomp(u8* dst, u32 dst_size, const u8* src, u32 src_size)
{
	u8 buf[1024];
	u8* pdst = dst;
	const u8* psrc = src + 12;

	if (*(const u32*)src != 0xFF435031)
		return;

	for (int i = 0; i < 256; i++)
	{
		buf[i] = i;
	}

	const u8 x = src[4];

	while ((psrc - src) < ((int)src_size - 12))
	{
		memcpy(&buf[256], buf, 256);

		for (int d = 0; d <= 0xFF; )
		{
			int c = *psrc++;
			if (c > 0x7F)
			{
				d += c - 0x7F;
				c = 0;
			}
			
			if (d > 0xff)
				break;

			for (int i = c+1; i > 0; i--)
			{
				buf[256 + d] = *psrc++;
				
				if (buf[256 + d] != d)
					buf[512 + d] = *psrc++;
				
				d++;
			}
		}

		int c;
		if (x & 1)
		{
			std::memcpy(&c, psrc, sizeof(u16));
			psrc += sizeof(u16);
		}
		else
		{
			std::memcpy(&c, psrc, sizeof(u32));
			psrc += sizeof(u32);
		}

		for (int n = 0; ; )
		{
			int d;
			if (n)
			{
				d = buf[--n + 768];
			}
			else
			{
				if (c == 0)
					break;
					
				c--;
				d = *psrc++;
			}
			
			if (buf[256 + d] == d)
			{
				if (pdst - dst > (int)dst_size)
					return;
				
				*pdst++ = d;
			}
			else
			{
				buf[768 + n++] = buf[512 + d];
				buf[768 + n++] = buf[256 + d];
			}
		}
	}
}

// padw, crc_or_something, DecryptFileNameV3, DecryptV3は、
// asmodean氏が作成・公開しているexfp3のソースコードを参考にして作成しました。

// Emulate mmx padw instruction
u64 CQLIE::padw(u64 a, u64 b)
{
	const u16* a_words = reinterpret_cast<u16*>(&a);
	const u16* b_words = reinterpret_cast<u16*>(&b);

	u64 ret = 0;
	u16* r_words = reinterpret_cast<u16*>(&ret);

	r_words[0] = a_words[0] + b_words[0];
	r_words[1] = a_words[1] + b_words[1];
	r_words[2] = a_words[2] + b_words[2];
	r_words[3] = a_words[3] + b_words[3];

	return ret;
}

uint32_t CQLIE::crc_or_something(const u8* buff, size_t len)
{
	u64 key = 0;
	u64 result = 0;
	const u64* p   = reinterpret_cast<const u64*>(buff);
	const u64* end = p + (len / 8);

	while (p < end)
	{
		key = padw(key, 0x0307030703070307);

		const u64 temp = *p++ ^ key;

		result = padw(result, temp);
	}

	result ^= (result >> 32);

	return static_cast<u32>(result & 0xFFFFFFFF);
}

void CQLIE::DecryptFileNameV3(u8* buff, u32 len, u32 seed)
{
	const u32 mutator = seed ^ 0x3E;
	const u32 key     = (mutator + len) & 0xFF;

	for (int i = 1; i <= (int)len; i++)
	{
		buff[i - 1] ^= ((i ^ key) & 0xFF) + i;
	}
}

void CQLIE::DecryptV3(u8* buff, u32 len, u32 seed)
{
	u64  key       = 0xA73C5F9DA73C5F9D;
	u32* key_words = (u32*)&key;
	u64  mutator   = (seed + len) ^ 0xFEC9753E;

	mutator = (mutator << 32) | mutator;

	u64* p = (u64*)buff;
	const u64* end = p + (len / 8);

	while (p < end)
	{
		// Emulate mmx padd instruction
		key_words[0] += 0xCE24F523;
		key_words[1] += 0xCE24F523;

		key     ^= mutator;
		mutator  = *p++ ^= key;
	}
}
