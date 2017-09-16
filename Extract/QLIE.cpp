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
	u32 ctFile;
	archive->ReadU32(&ctFile);

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
		for (int i = 0; i < (int)ctFile; i++)
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

	for (u32 i = 0; i < ctFile; i++)
	{
		// Get the filename length
		const u16 FileNameLen = *(const u16*)&pIndex[0];
		
		//archive->Read(&FileNameLen, 2);
		u8 szFileName[_MAX_FNAME];
		memcpy(szFileName, &pIndex[2], FileNameLen);
		
		//archive->Read(szFileName, FileNameLen);
		szFileName[FileNameLen] = '\0';
		
		// Get the filename
		DecryptFunc(szFileName, FileNameLen, seed);

		pIndex += 2 + FileNameLen;

		//u8 index[28];
		//archive->Read(&index, 28);

		// Add to the listview.
		SFileInfo infFile;
		infFile.name = (char*)szFileName;
		infFile.start = *(const u64*)&pIndex[0];
		infFile.sizeCmp = *(const u32*)&pIndex[8];
		infFile.sizeOrg = *(const u32*)&pIndex[12];
		infFile.end = infFile.start + infFile.sizeCmp;
		infFile.key = (version == _T("FilePackVer1.0")) ? *(const u32*)&pIndex[20] : seed;
		infFile.title = version;
		archive->AddFileInfo(infFile);

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
		const LPBITMAPFILEHEADER fHed = (LPBITMAPFILEHEADER)&pBuf[0];
		const LPBITMAPINFOHEADER iHed = (LPBITMAPINFOHEADER)&pBuf[14];

		// Output size
		u32 dstSize = file_info->sizeOrg - 54;

		if (((iHed->biWidth * (iHed->biBitCount >> 3) + 3) & 0xFFFFFFFC) * iHed->biHeight != dstSize)
			dstSize -= 2;

		// Output
		CImage image;
		image.Init(archive, iHed->biWidth, iHed->biHeight, iHed->biBitCount, &pBuf[54], fHed->bfOffBits - 54);
		image.Write(&pBuf[fHed->bfOffBits], dstSize);
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
	u32 dwSrcIndex = 0;

	if ((dwSrcIndex + 16) > src_size)
	{
		// Exit
		return false;
	}

	if (memcmp(&src[dwSrcIndex], "ABMP7", 5) != 0)
	{
		// Unsupported
		return false;
	}

	dwSrcIndex += 12;

	// Get the amount of unnecessary data we can skip
	const u32 dwUnKnownDataSize = *(u32*)&src[dwSrcIndex];
	dwSrcIndex += 4;

	// Skip the unnecessary data
	dwSrcIndex += dwUnKnownDataSize;

	//-- First read ------------------------------------------------------------------------

	// Get filesize
	u32 dwFileSize = *(u32*)&src[dwSrcIndex];
	dwSrcIndex += 4;

	// Get the file extension
	YCString clsFileExt = GetExtension(&src[dwSrcIndex]);

	// Output
	if (clsFileExt == _T(".bmp"))
	{
		// BITMAP

		CImage clImage;
		clImage.Init(archive, &src[dwSrcIndex]);
		clImage.Write(dwFileSize);
		clImage.Close();
	}
	else
	{
		// Other

		archive->OpenFile(clsFileExt);
		archive->WriteFile(&src[dwSrcIndex], dwFileSize);
		archive->CloseFile();
	}

	dwSrcIndex += dwFileSize;

	//-- Second read (and subsequent reads) -----------------------------------------------------------------

	while (dwSrcIndex < src_size)
	{
		// Get the length of the filename

		const u32 dwFileNameLength = src[dwSrcIndex];

		dwSrcIndex += 1;

		// Get the filename
		char szFileName[_MAX_FNAME];
		memcpy(szFileName, &src[dwSrcIndex], dwFileNameLength);
		szFileName[dwFileNameLength] = '\0';

		dwSrcIndex += 31;

		// Get the file size
		dwFileSize = *(u32*)&src[dwSrcIndex];
		dwSrcIndex += 4;

		if (dwFileSize == 0)
		{
			continue;
		}

		// Get file extension

		clsFileExt = GetExtension(&src[dwSrcIndex]);

		// Write the .b filename extension

		TCHAR szWork[_MAX_FNAME];
		_stprintf(szWork, _T("_%s%s"), szFileName, clsFileExt.GetString());

		// Output

		if (clsFileExt == _T(".bmp"))
		{
			// BITMAP

			CImage clImage;
			clImage.Init(archive, &src[dwSrcIndex], szWork);
			clImage.Write(dwFileSize);
			clImage.Close();
		}
		else
		{
			// Other

			archive->OpenFile(szWork);
			archive->WriteFile(&src[dwSrcIndex], dwFileSize);
			archive->CloseFile();
		}

		dwSrcIndex += dwFileSize;
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
	static u32 dwDstFiles;
	static std::vector<FileNameInfo> vtFileNameList;

	if (src_index_ptr == nullptr)
	{
		// First call
		dwDstFiles = 0;
		vtFileNameList.clear();
	}

	u32 dwSrcIndex = 0;

	if ((dwSrcIndex + 16) > src_size)
	{
		// Exit
		return false;
	}

	if (memcmp(&src[dwSrcIndex], "abmp", 4) != 0)
	{
		// Unsupported
		return false;
	}

	dwSrcIndex += 16;

	if (memcmp(&src[dwSrcIndex], "abdata", 6) != 0)
	{
		// Unsupported
		return false;
	}

	dwSrcIndex += 16;

	// Get the amount of data we can skip
	const u32 dwUnKnownDataSize = *(u32*)&src[dwSrcIndex];
	dwSrcIndex += 4;

	// Skip the unnecessary data
	dwSrcIndex += dwUnKnownDataSize;

	while (dwSrcIndex < src_size)
	{
		if ((dwSrcIndex + 16) > src_size)
		{
			// Exit
			break;
		}

		if (memcmp(&src[dwSrcIndex], "abimage", 7) == 0)
		{
			// abimage10
		}
		else if (memcmp(&src[dwSrcIndex], "absound", 7) == 0)
		{
			// absound10
		}
		else
		{
			// Unsupported
			return false;
		}

		dwSrcIndex += 16;

		// Get file count
		const u32 dwFiles = src[dwSrcIndex];
		dwSrcIndex += 1;

		for (u32 i = 0; i < dwFiles; i++)
		{
			if ((dwSrcIndex + 16) > src_size)
			{
				// Exit
				break;
			}

			u32 dwDatVersion;

			if (memcmp(&src[dwSrcIndex], "abimgdat10", 10) == 0)
			{
				// abimgdat10
				dwDatVersion = ABIMGDAT10;
			}
			else if (memcmp(&src[dwSrcIndex], "abimgdat11", 10) == 0)
			{
				// abimgdat11
				dwDatVersion = ABIMGDAT11;
			}
			else if (memcmp(&src[dwSrcIndex], "absnddat10", 10) == 0)
			{
				// absnddat10
				dwDatVersion = ABSNDDAT10;
			}
			else if (memcmp(&src[dwSrcIndex], "absnddat11", 10) == 0)
			{
				// absnddat11
				dwDatVersion = ABSNDDAT11;
			}
			else
			{
				// Unsupported
				return false;
			}

			dwSrcIndex += 16;

			// Get the length of the filename
			const u32 dwFileNameLength = *(u16*)&src[dwSrcIndex];
			dwSrcIndex += 2;

			// Get the filename
			YCString clsFileName((char*)&src[dwSrcIndex], dwFileNameLength);
			dwSrcIndex += dwFileNameLength;

			// Skip unknown data

			if ((dwDatVersion == ABIMGDAT11) || (dwDatVersion == ABSNDDAT11))
			{
				const u16 wLength = *(u16*)&src[dwSrcIndex];

				dwSrcIndex += 2 + wLength;
			}

			dwSrcIndex += 1;

			// Get file size
			const u32 dwFileSize = *(u32*)&src[dwSrcIndex];
			dwSrcIndex += 4;

			if (dwFileSize == 0)
			{
				// File size is 0

				continue;
			}

			// Get the file extension
			const YCString clsFileExt = GetExtension(&src[dwSrcIndex]);

			if (clsFileExt == _T(".b"))
			{
				// abmp10

				YCString clsWork;

				if (clsFileName == _T(""))
				{
					clsWork = b_file_name;
				}
				else
				{
					clsWork = b_file_name + _T("_") + clsFileName;
				}

				DecodeABMP10(archive, &src[dwSrcIndex], dwFileSize, &dwSrcIndex, clsWork);

				continue;
			}

			// Replace '/' with '_' (Taking into account double-byte characters)
			clsFileName.Replace(_T('/'), _T('_'));

			// Erase illegal chars
			EraseNotUsePathWord(clsFileName);

			// Rename extension
			clsFileName.RenameExtension(clsFileExt); // Renaming because there may be a psd file with the same extension

			// Write the .b extension
			TCHAR szWork[_MAX_FNAME];

			if (dwFileNameLength == 0)
			{
				// No filename (only put extension)

				_stprintf(szWork, _T("%s%s"), b_file_name.GetString(), clsFileName.GetString());
			}
			else
			{
				_stprintf(szWork, _T("%s_%s"), b_file_name.GetString(), clsFileName.GetString());
			}

			// Check to see if the same filename doesn't exist

			for (size_t uIndex = 0; uIndex < vtFileNameList.size(); uIndex++)
			{
				if (vtFileNameList[uIndex].file_name == szWork)
				{
					// Same filename

					vtFileNameList[uIndex].count++;

					// Overwrite will not occur

					TCHAR szWork2[256];
					_stprintf(szWork2, _T("_%d%s"), vtFileNameList[uIndex].count, clsFileName.GetFileExt().GetString());

					PathRenameExtension(szWork, szWork2);

					// Reset the loop counter to ensure the same file does not exist now.

					uIndex = (size_t)-1;
				}
			}

			// Register the filename

			FileNameInfo file_name_info;
			file_name_info.file_name = szWork;
			file_name_info.count = 1;

			vtFileNameList.push_back(file_name_info);

			// Output

			if (clsFileExt == _T(".bmp"))
			{
				// BITMAP

				CImage clImage;
				clImage.Init(archive, &src[dwSrcIndex], szWork);
				clImage.Write(dwFileSize);
				clImage.Close();
			}
			else
			{
				// Other

				archive->OpenFile(szWork);
				archive->WriteFile(&src[dwSrcIndex], dwFileSize);
				archive->CloseFile();
			}

			dwDstFiles++;

			dwSrcIndex += dwFileSize;
		}
	}

	if (src_index_ptr != nullptr)
	{
		// Recursive call being performed

		*src_index_ptr += dwSrcIndex;
	}
	else
	{
		if (dwDstFiles == 0)
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
/// @param clsPath Path
///
void CQLIE::EraseNotUsePathWord(YCString& path)
{
	static const TCHAR aszNotUsePathWord[] =
	{
		_T(':'), _T(','), _T(';'), _T('*'), _T('?'), _T('\"'), _T('<'), _T('>'), _T('|')
	};

	for (const TCHAR& c : aszNotUsePathWord)
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
