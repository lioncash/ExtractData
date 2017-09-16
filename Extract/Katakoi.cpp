#include "StdAfx.h"
#include "Extract/Katakoi.h"

#include "ArcFile.h"
#include "File.h"
#include "Image.h"
#include "Sound/Ogg.h"
#include "Sound/Wav.h"

bool CKatakoi::Mount(CArcFile* archive)
{
	if (MountIar(archive))
		return true;

	if (MountWar(archive))
		return true;

	return false;
}

bool CKatakoi::MountIar(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".iar"))
		return false;

	if (memcmp(archive->GetHeader(), "iar ", 4) != 0)
		return false;

	// Version check
	const DWORD version = *(LPDWORD)&archive->GetHeader()[4];
	DWORD file_entry_size = 0;

	if (version == 2)
	{
		file_entry_size = 4;
	}
	else if (version == 3)
	{
		file_entry_size = 8;
	}
	else
	{
		return false;
	}

	archive->SeekHed(0x1C);

	// Get number of files
	DWORD num_files;
	archive->Read(&num_files, 4);

	// Get index size
	const DWORD index_size = num_files * file_entry_size;

	// Get index
	std::vector<BYTE> index(index_size);
	archive->Read(index.data(), index.size());

	// Get index filename
	std::vector<BYTE> sec;
	u32 name_index;

	const bool found_sec = GetNameIndex(archive, sec, name_index);

	// File information retrieval
	TCHAR file_name[_MAX_FNAME];
	TCHAR work[_MAX_FNAME];

	if (!found_sec)
	{
		// Failed to get the filename index
		lstrcpy(work, archive->GetArcName());
		PathRemoveExtension(work);
	}

	for (DWORD i = 0; i < num_files; i++)
	{
		if (!found_sec)
		{
			// Create a sequential filename
			_stprintf(file_name, _T("%s_%06u"), work, i);
		}
		else
		{
			// Get the name of the file from the filename index
			lstrcpy(file_name, (LPCTSTR)&sec[name_index]);

			name_index += strlen((char*)&sec[name_index]) + 1; // Filename
			name_index += strlen((char*)&sec[name_index]) + 1; // File type
			name_index += strlen((char*)&sec[name_index]) + 1; // Archive type
			name_index += 4 + *(LPDWORD)&sec[name_index];      // Archive name length + Archive name + File number
		}

		SFileInfo info;
		info.name = file_name;
		info.start = *(LPDWORD)&index[i * file_entry_size];
		info.end = ((i + 1) < num_files) ? *(LPDWORD)&index[(i+1) * file_entry_size] : archive->GetArcSize();
		info.sizeCmp = info.end - info.start;
		info.sizeOrg = info.sizeCmp;

		archive->AddFileInfo(info);
	}

	return true;
}

bool CKatakoi::MountWar(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".war"))
		return false;

	if (memcmp(archive->GetHeader(), "war ", 4) != 0)
		return false;

	// Version check
	const DWORD version = *(LPDWORD)&archive->GetHeader()[4];
	DWORD file_entry_size = 0;

	if (version == 8)
	{
		file_entry_size = 24;
	}
	else
	{
		return false;
	}

	archive->SeekHed(0x08);

	// Get the number of files
	DWORD num_files;
	archive->Read(&num_files, 4);

	// Get index size
	const DWORD index_size = num_files * file_entry_size;

	// Get index
	std::vector<BYTE> index(index_size);
	archive->SeekCur(0x04);
	archive->Read(index.data(), index.size());

	// Get the filename index
	std::vector<BYTE> sec;
	u32 name_index;

	const bool found_sec = GetNameIndex(archive, sec, name_index);

	// Set index for each archive filename to see if it is correct (used in delta synthesis/decoding)
	archive->SetFlag(found_sec);

	// Getting file info
	TCHAR file_name[_MAX_FNAME];
	TCHAR work[_MAX_FNAME];

	if (!found_sec)
	{
		// Failed to get the filename index
		lstrcpy(work, archive->GetArcName());
		PathRemoveExtension(work);
	}

	for (DWORD i = 0; i < num_files; i++)
	{
		if (!found_sec)
		{
			// Create a sequential filename
			_stprintf(file_name, _T("%s_%06u"), work, i);
		}
		else
		{
			// Get filename from the filename index
			lstrcpy(file_name, (LPCTSTR)&sec[name_index]);

			name_index += strlen((char*)&sec[name_index]) + 1; // File name
			name_index += strlen((char*)&sec[name_index]) + 1; // File type
			name_index += strlen((char*)&sec[name_index]) + 1; // Archive type
			name_index += 4 + *(LPDWORD)&sec[name_index];      // Archive name length + Archive name + File number
		}

		SFileInfo info;
		info.name = file_name;
		info.start = *(LPDWORD)&index[i * file_entry_size];
		info.sizeCmp = *(LPDWORD)&index[i * file_entry_size + 4];
		info.sizeOrg = info.sizeCmp;
		info.end = info.start + info.sizeCmp;
		archive->AddFileInfo(info);
	}

	return true;
}

bool CKatakoi::GetNameIndex(CArcFile* archive, std::vector<u8>& sec, u32& name_index)
{
	// Open the filename represented by the index
	TCHAR sec_path[MAX_PATH];

	if (!GetPathToSec(sec_path, archive->GetArcPath()))
	{
		// SEC5 file couldn't be found
		return false;
	}

	CFile sec_file;

	if (!sec_file.OpenForRead(sec_path))
	{
		// Failed to open the sec5 file
		return false;
	}

	const DWORD sec_size = sec_file.GetFileSize();

	// Reading
	sec.resize(sec_size);
	sec_file.Read(sec.data(), sec.size());

	if (memcmp(sec.data(), "SEC5", 4) != 0)
	{
		// Incorrect SEC5 file
		return false;
	}

	// Find the RESR
	for (name_index = 8; name_index < sec_size; )
	{
		if (memcmp(&sec[name_index], "RESR", 4) == 0)
		{
			// Found "RESR"
			const DWORD name_index_size = *(LPDWORD)&sec[name_index + 4];
			const DWORD num_name_index_files = *(LPDWORD)&sec[name_index + 8];

			name_index += 12;

			// Find the index that matches the name of the archive
			for (DWORD i = 0; i < num_name_index_files; i++)
			{
				DWORD work = 0;
				work += strlen((char*)&sec[name_index + work]) + 1; // File name
				work += strlen((char*)&sec[name_index + work]) + 1; // File type
				work += strlen((char*)&sec[name_index + work]) + 1; // Archive type

				const DWORD length = *(LPDWORD)&sec[name_index + work]; // Archive name + File number
				work += 4;

				for (DWORD j = name_index + work; ; j++)
				{
					if (sec[j] == '\0')
					{
						// Index doesn't match the name of the archive
						break;
					}

					if (lstrcmp((LPCTSTR)&sec[j], archive->GetArcName()) == 0)
					{
						// Found a match with the name of the archive
						if (lstrcmp(PathFindFileName(sec_path), _T("toa.sec5")) == 0)
						{
							// 杏奈ちゃんにお願い
							archive->SetFlag(true);
						}
						else if (lstrcmp(PathFindFileName(sec_path), _T("katakoi.sec5")) == 0)
						{
							// 片恋いの月
							archive->SetFlag(true);
						}

						return true;
					}
				}

				name_index += work + length;
			}
			break;
		}

		name_index += 8 + *(LPDWORD)&sec[name_index + 4];
	}

	// No file in the index was a match
	return false;
}

bool CKatakoi::GetPathToSec(LPTSTR sec_path, const YCString& archive_path)
{
	TCHAR work[MAX_PATH];

	lstrcpy(work, archive_path);
	PathRemoveFileSpec(work);
	PathAppend(work, _T("*.sec5"));

	// Locate the sec5 file from within the archive folder.
	WIN32_FIND_DATA find_data;
	HANDLE find_handle = FindFirstFile(work, &find_data);

	if (find_handle == INVALID_HANDLE_VALUE)
	{
		// Locate the sec5 file from the installation folder (hopefully)
		PathRemoveFileSpec(work);
		PathRemoveFileSpec(work);
		PathAppend(work, _T("*.sec5"));

		find_handle = FindFirstFile(work, &find_data);

		if (find_handle == INVALID_HANDLE_VALUE)
		{
			// sec5 file couldn't be found
			return false;
		}
	}

	FindClose(find_handle);

	lstrcpy(sec_path, work);
	PathRemoveFileSpec(sec_path);
	PathAppend(sec_path, find_data.cFileName);

	return true;
}

bool CKatakoi::Decode(CArcFile* archive)
{
	if (DecodeIar(archive))
		return true;

	if (DecodeWar(archive))
		return true;

	return false;
}

bool CKatakoi::DecodeIar(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".iar"))
		return false;

	if (memcmp(archive->GetHeader(), "iar ", 4) != 0)
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Reading
	std::vector<BYTE> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	// Output buffer
	const DWORD dst_size = *(LPDWORD)&src[8];
	std::vector<BYTE> dst(dst_size * 2);

	// Decompression
	DecompImage(dst.data(), dst_size, &src[64], *(LPDWORD)&src[16]);

	const long width = *(LPLONG)&src[32];
	const long height = *(LPLONG)&src[36];
	WORD bpp;

	switch (src[0])
	{
		case 0x02:
			bpp = 8;
			break;
		case 0x1C:
			bpp = 24;
			break;
		case 0x3C:
			bpp = 32;
			break;
		default:
			return false;
	}

	const bool is_delta_file = src[1] == 8;

	if (is_delta_file)
	{
		// Difference file
		DecodeCompose(archive, dst.data(), dst_size, width, height, bpp);
	}
	else
	{
		CImage image;
		image.Init(archive, width, height, bpp);
		image.WriteReverse(dst.data(), dst_size);
	}

	return true;
}

bool CKatakoi::DecodeWar(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".war"))
		return false;

	if (memcmp(archive->GetHeader(), "war ", 4) != 0)
		return false;

	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Reading
	std::vector<BYTE> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	if (memcmp(src.data(), "OggS", 4) == 0)
	{
		// Ogg Vorbis
		COgg ogg;
		ogg.Decode(archive, src.data());
	}
	else
	{
		// WAV (supposedly)
		const DWORD data_size = *(LPDWORD)&src[4];
		const DWORD frequency = *(LPDWORD)&src[12];
		const WORD  channels = *(LPWORD)&src[10];
		const WORD  bits = *(LPWORD)&src[22];

		CWav wav;
		wav.Init(archive, data_size, frequency, channels, bits);
		wav.Write(&src[24]);
	}

	return true;
}

void CKatakoi::GetBit(const u8*& src, u32& flags)
{
	flags >>= 1;

	if (flags <= 0xFFFF)
	{
		// Less than or equal to 0xFFFF

		flags = *(LPWORD)&src[0] | 0xFFFF0000;
		src += 2;
	}
}

bool CKatakoi::DecompImage(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	u32 flags = 0; // Flag can always be initialized
	DWORD back;
	DWORD length;

	const u8* dst_begin = dst;
	const u8* src_end = src + src_size;
	const u8* dst_end = dst + dst_size;

	while (src < src_end && dst < dst_end)
	{
		GetBit(src, flags);

		// Uncompressed data
		if (flags & 1)
		{
			*dst++ = *src++;
		}
		else // Compressed data
		{
			GetBit(src, flags);

			if (flags & 1)
			{
				// Compression pattern 1 (3 or more compressed bytes)

				// Determine the number of bytes to return
				GetBit(src, flags);

				// Plus one byte
				DWORD dwWork = 1;
				back = flags & 1;

				GetBit(src, flags);

				if ((flags & 1) == 0)
				{
					// Plus 0x201 bytes
					GetBit(src, flags);
					dwWork = 0x201;

					if ((flags & 1) == 0)
					{
						// Plus 0x401 bytes
						GetBit(src, flags);

						dwWork = 0x401;
						back = (back << 1) | (flags & 1);

						GetBit(src, flags);

						if ((flags & 1) == 0)
						{
							// Plus 0x801 bytes
							GetBit(src, flags);

							dwWork = 0x801;
							back = (back << 1) | (flags & 1);

							GetBit(src, flags);

							if ((flags & 1) == 0)
							{
								// Plus 0x1001 bytes
								GetBit(src, flags);

								dwWork = 0x1001;
								back = (back << 1) | (flags & 1);
							}
						}
					}
				}

				back = ((back << 8) | *src++) + dwWork;

				// Determine the number of compressed bytes
				GetBit(src, flags);

				if (flags & 1)
				{
					// 3 bytes of compressed data
					length = 3;
				}
				else
				{
					GetBit(src, flags);

					if (flags & 1)
					{
						// 4 bytes of compressed data
						length = 4;
					}
					else
					{
						GetBit(src, flags);

						if (flags & 1)
						{
							// 5 bytes of compressed data
							length = 5;
						}
						else
						{
							GetBit(src, flags);

							if (flags & 1)
							{
								// 6 bytes of compressed data
								length = 6;
							}
							else
							{
								GetBit(src, flags);

								if (flags & 1)
								{
									// 7~8 bytes of compressed data
									GetBit(src, flags);

									length = (flags & 1);
									length += 7;
								}
								else
								{
									GetBit(src, flags);

									if (flags & 1)
									{
										// More than 17 bytes of compressed data
										length = *src++ + 0x11;
									}
									else
									{
										// 9~16 bytes of compressed data
										GetBit(src, flags);
										length = (flags & 1) << 2;

										GetBit(src, flags);
										length |= (flags & 1) << 1;

										GetBit(src, flags);
										length |= (flags & 1);

										length += 9;
									}
								}
							}
						}
					}
				}
			}
			else
			{
				// Compression pattern 2 (Compressed data is 2 bytes)
				length = 2;

				// Determine the number of bytes to return
				GetBit(src, flags);

				if (flags & 1)
				{
					GetBit(src, flags);
					back = (flags & 1) << 0x0A;

					GetBit(src, flags);
					back |= (flags & 1) << 0x09;

					GetBit(src, flags);
					back |= (flags & 1) << 0x08;

					back |= *src++;
					back += 0x100;
				}
				else
				{
					back = *src++ + 1;

					if (back == 0x100)
					{
						// Exit
						break;
					}
				}
			}

			// Decompress compressed files
			if (back > dst - dst_begin)
			{
				return false;
			}

			LPBYTE dst_copy_ptr = dst - back;

			for (DWORD k = 0; k < length && dst < dst_end && dst_copy_ptr < dst_end; k++)
			{
				*dst++ = *dst_copy_ptr++;
			}
		}
	}

	return true;
}

bool CKatakoi::DecodeCompose(CArcFile* archive, const u8* diff, size_t diff_size, long diff_width, long diff_height, u16 diff_bpp)
{
	const SFileInfo* diff_file_info = archive->GetOpenFileInfo();

	const SFileInfo* base_file_info = nullptr;
	BOOL             base_file_exists = FALSE;
	TCHAR            base_file_name[MAX_PATH];

	lstrcpy(base_file_name, diff_file_info->name);

	LPTSTR diff_file_number_str1 = &base_file_name[lstrlen(base_file_name) - 1];
	LPTSTR diff_file_number_str2 = &base_file_name[lstrlen(base_file_name) - 2];

	// Convert numerical value to a serial number
	const long diff_file_number1 = _tcstol(diff_file_number_str1, nullptr, 10);
	const long diff_file_number2 = _tcstol(diff_file_number_str2, nullptr, 10);

	if (archive->GetFlag())
	{
		// Base file search (search from delta file)
		long base_file_number = diff_file_number1;
		long count = diff_file_number1;

		while (!base_file_exists)
		{
			base_file_number--;
			count--;

			if (count < 0)
			{
				// End search
				break;
			}

			_stprintf(diff_file_number_str1, _T("%d"), base_file_number);

			base_file_info = archive->GetFileInfo(base_file_name);

			if (base_file_info == nullptr)
			{
				// Missing number file
				continue;
			}

			BYTE work;
			archive->SeekHed(base_file_info->start + 1);
			archive->Read(&work, 1);

			if (work == 0)
			{
				// Found base file
				base_file_exists = TRUE;
			}
		}

		// Base file search (find difference after the file)
		base_file_number = diff_file_number1;
		count = diff_file_number1;

		while (!base_file_exists)
		{
			base_file_number++;
			count++;

			if (count >= 10)
			{
				// End search
				break;
			}

			_stprintf(diff_file_number_str1, _T("%d"), base_file_number);

			base_file_info = archive->GetFileInfo(base_file_name);

			if (base_file_info == nullptr)
			{
				// Missing number file
				continue;
			}

			BYTE work;
			archive->SeekHed(base_file_info->start + 1);
			archive->Read(&work, 1);

			if (work == 0)
			{
				// Found base file
				base_file_exists = TRUE;
			}
		}

		// Base file search (2桁目を1つ戻して検索)
		base_file_number = (diff_file_number2 / 10) * 10;
		count = 10;

		while (!base_file_exists)
		{
			base_file_number--;
			count--;

			if (count < 0)
			{
				// End search
				break;
			}

			_stprintf(diff_file_number_str2, _T("%02d"), base_file_number);

			base_file_info = archive->GetFileInfo(base_file_name);

			if (base_file_info == nullptr)
			{
				// Missing number file
				continue;
			}

			BYTE work;
			archive->SeekHed(base_file_info->start + 1);
			archive->Read(&work, 1);

			if (work == 0)
			{
				// Found base file
				base_file_exists = TRUE;
			}
		}
	}

	if (base_file_exists)
	{
		// Base file exists
		std::vector<BYTE> base_src(base_file_info->sizeCmp);
		archive->SeekHed(base_file_info->start);
		archive->Read(base_src.data(), base_src.size());

		const long base_width = *(LPLONG)&base_src[32];
		const long base_height = *(LPLONG)&base_src[36];

		if (base_width >= diff_width && base_height >= diff_height)
		{
			// Large base
			const DWORD       base_dst_size = *(LPDWORD)&base_src[8];
			std::vector<BYTE> base_dst(base_dst_size);

			// Decompress base file
			DecompImage(base_dst.data(), base_dst.size(), &base_src[64], *(LPDWORD)&base_src[16]);

			// Synthesize base file and delta file
			Compose(base_dst.data(), base_dst.size(), diff, diff_size, base_width, diff_width, diff_bpp);

			// Output
			const long width = *(LPLONG)&base_src[32];
			const long height = *(LPLONG)&base_src[36];

			CImage image;
			image.Init(archive, width, height, diff_bpp);
			image.WriteReverse(base_dst.data(), base_dst.size());

			return true;
		}
		else if (diff_width >= base_width && diff_height >= base_height)
		{
			// Difference is greater
			const DWORD       base_dst_size = *(LPDWORD)&base_src[8];
			std::vector<BYTE> base_dst(base_dst_size);

			// Decompress base file
			DecompImage(base_dst.data(), base_dst.size(), &base_src[64], *(LPDWORD)&base_src[16]);

			// The difference in the size of the memory allocation
			const DWORD       dst_size = diff_width * diff_height * (diff_bpp >> 3);
			std::vector<BYTE> dst(dst_size);

			// Align base file in the lower-right
			const long start_x = diff_width - base_width;
			const long start_y = diff_height - base_height;
			LPBYTE base_dst_ptr = base_dst.data();
			LPBYTE dst_ptr = dst.data();

			const long x_gap = start_x * (diff_bpp >> 3);
			const long base_line = base_width * (diff_bpp >> 3);
			const long diff_line = diff_width * (diff_bpp >> 3);

			// Fit under the vertical position
			dst_ptr += start_y * diff_line;

			for (long y = start_y; y < diff_height; y++)
			{
				// According to the horizontal position to the right.
				dst_ptr += x_gap;

				memcpy(dst_ptr, base_dst_ptr, base_line);

				dst_ptr += base_line;
				base_dst_ptr += base_line;
			}

			// Synthesize the base file and the delta file
			Compose(dst.data(), dst.size(), diff, diff_size, diff_width, diff_width, diff_bpp);

			// Output
			CImage image;
			image.Init(archive, diff_width, diff_height, diff_bpp);
			image.WriteReverse(dst.data(), dst.size());

			return true;
		}
	}

	// Base file doesn't exist.
	// Prepare black data
	const DWORD dst_size = ((diff_width * (diff_bpp >> 3) + 3) & 0xFFFFFFFC) * diff_height;
	std::vector<BYTE> dst(dst_size);

	// Synthesize black data
	Compose(dst.data(), dst.size(), diff, diff_size, diff_width, diff_width, diff_bpp);

	CImage image;
	image.Init(archive, diff_width, diff_height, diff_bpp);
	image.WriteReverse(dst.data(), dst.size());
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Synthesizes the base image and the difference image.
//
// 十一寒月氏が作成・公開しているiarのソースコードを参考にして作成しました。
bool CKatakoi::Compose(u8* dst, size_t dst_size, const u8* src, size_t src_size, long dst_width, long src_width, u16 bpp)
{
	const u16 colors = bpp >> 3;
	const u32 line = src_width * colors;
	DWORD height = *(LPDWORD)&src[8];

	DWORD x_gap = 0;

	if (dst_width > src_width)
	{
		x_gap = (dst_width - src_width) * colors;
	}

	size_t src_idx = 12;
	size_t dst_idx = *(LPDWORD)&src[4] * (x_gap + line);

	while (height-- && src_idx < src_size)
	{
		for (DWORD x = 0; x < x_gap; x++)
		{
			dst[dst_idx++] = 0;
		}

		DWORD count = *(LPWORD)&src[src_idx];
		src_idx += 2;

		size_t offset = 0;

		while (count--)
		{
			offset += *(LPWORD)&src[src_idx] * colors;
			src_idx += 2;

			DWORD length = *(LPWORD)&src[src_idx] * colors;
			src_idx += 2;

			while (length--)
			{
				dst[dst_idx + offset++] = src[src_idx++];

				if (dst_idx + offset >= dst_size)
				{
					return true;
				}

				if (src_idx >= src_size)
				{
					return true;
				}
			}
		}

		dst_idx += line;
	}

	return true;
}
