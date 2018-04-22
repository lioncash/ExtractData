#include "StdAfx.h"
#include "Extract/Alcot.h"

#include "ArcFile.h"
#include "Image.h"
#include "Arc/Zlib.h"

bool CAlcot::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".arc") || memcmp(archive->GetHeader(), "ARC\x1a", 4) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->Seek(8, FILE_BEGIN);
	archive->ReadU32(&num_files);

	// Get compressed index size
	u32 compressed_index_size;
	archive->ReadU32(&compressed_index_size);

	// Get compressed index
	std::vector<u8> compressed_index(compressed_index_size);
	archive->Seek(0x30, FILE_BEGIN);
	archive->Read(compressed_index.data(), compressed_index.size());

	// Get index size
	const size_t index_size = *(const u32*)&compressed_index[16];

	// Decompress the index
	std::vector<u8> index(index_size);
	Decomp(index.data(), index.size(), compressed_index.data());

	const size_t offset = compressed_index_size + 0x30;
	const size_t size = index_size / num_files;
	const size_t file_name_length = size - 16;

	const u8* index_ptr = index.data();
	for (u32 i = 0; i < num_files; i++)
	{
		// Get filename
		TCHAR file_name[48];
		memcpy(file_name, &index_ptr[16], file_name_length);

		// Add to the listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.sizeOrg = *(const u32*)&index_ptr[4];
		file_info.sizeCmp = file_info.sizeOrg;
		file_info.start = *(const u32*)&index_ptr[0] + offset;
		file_info.end = file_info.start + file_info.sizeOrg;
		archive->AddFileInfo(file_info);

		index_ptr += size;
	}

	return true;
}

bool CAlcot::Decode(CArcFile* archive)
{
	if (DecodeASB(archive))
		return true;

	if (DecodeCPB(archive))
		return true;

	return false;
}

bool CAlcot::DecodeASB(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("ASB"))
		return false;

	// Reading
	std::vector<u8> z_buf(file_info->sizeCmp);
	archive->Read(z_buf.data(), z_buf.size());

	// Get size
	const u32 z_buf_len = *(u32*)&z_buf[4];
	const u32 buf_len = *(u32*)&z_buf[8];

	// Decryption
	Decrypt(&z_buf[12], z_buf_len, buf_len);

	// ASB not supported, lets force support
	if (memcmp(&z_buf[16], "\x78\xDA", 2) != 0)
	{
		const u32 key = ((*(u32*)&z_buf[16] - 0x78) ^ buf_len) & 0xFF;
		std::vector<u8> tmp(z_buf_len);

		for (u64 i = 0; i < 0xFFFFFFFFULL; i += 0x00000100ULL)
		{
			const u32 x = static_cast<u32>(i | key);
			memcpy(tmp.data(), &z_buf[16], z_buf_len);

			u32 key2 = buf_len ^ x;
			key2 ^= ((key2 << 0x0C) | key2) << 0x0B;

			const u32 tmp2 = *(u32*)&z_buf[16] - key2;
			if ((tmp2 & 0x0000FFFF) != 0xDA78)
			{
				continue;
			}

			for (size_t j = 0; j < z_buf_len / sizeof(u32); j++)
			{
				*(u32*)&tmp[j * sizeof(u32)] -= key2;
			}

			// Decompression
			CZlib zlib;
			std::vector<u8> buf(buf_len);
			zlib.Decompress(buf.data(), buf.size(), tmp.data(), z_buf_len);

			constexpr std::array<u8, 20> a{{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

			if (memcmp(&buf[buf_len - a.size()], a.data(), a.size()) == 0)
			{
				TCHAR mes[256];
				_stprintf(mes, _T("Decryption key of this game is %08X . \n We may be able to support this archive type if you report it to us"), x);
				MessageBox(archive->GetProg()->GetHandle(), mes, _T("Decryption Key"), MB_OK);

				_stprintf(mes, _T("_%08X.txt"), x);
				archive->OpenFile(mes);
				archive->WriteFile(buf.data(), buf.size());
				archive->CloseFile();
			}
		}
		return true;
	}

	// Decompression
	CZlib zlib;
	std::vector<u8> buf(buf_len);
	zlib.Decompress(buf.data(), buf.size(), &z_buf[16], z_buf_len);

	archive->OpenScriptFile();
	archive->WriteFile(buf.data(), buf.size());

	return true;
}

bool CAlcot::DecodeCPB(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("CPB"))
		return false;

	// Read
	std::vector<u8> z_buf(file_info->sizeCmp);
	archive->Read(z_buf.data(), z_buf.size());
	u8* z_pbuf = z_buf.data();

	const u8 type = z_buf[4];
	const u8 bpp = z_buf[5];
	const u8 colors = bpp >> 3;

	s32 width;
	s32 height;

	std::array<u32, 4> z_color_size;
	size_t buf_len;
	std::vector<u8> buf;

	// Triptych
	if (type == 0)
	{
		// Product version
		if (*(u16*)&z_buf[6] == 1)
			width = *(u16*)&z_buf[12];
		// Trial Version
		else if (*(u16*)&z_buf[6] == 0x100)
			width = *(u16*)&z_buf[8];
		else // Other
			return false;

		height = *(u16*)&z_buf[14];

		const u32 color_size = width * height;

		buf_len = color_size * colors;
		buf.resize(buf_len);
		u8* pbuf = buf.data();
		pbuf += buf_len;

		z_color_size[0] = *(u32*)&z_buf[16];
		z_color_size[1] = *(u32*)&z_buf[28];
		z_color_size[2] = *(u32*)&z_buf[20];
		z_color_size[3] = *(u32*)&z_buf[24];

		CZlib zlib;
		z_pbuf += 0x20;
		for (const u32 size : z_color_size)
		{
			if (size == 0)
				continue;

			pbuf -= color_size;
			zlib.Decompress(pbuf, color_size, &z_pbuf[4], size);
			z_pbuf += size;
		}
	}
	else
	{
		width = *(u16*)&z_buf[8];
		height = *(u16*)&z_buf[10];

		const u32 color_size = width * height;

		buf_len = color_size * colors;
		buf.resize(buf_len);
		u8* pbuf = buf.data();

		for (size_t i = 0; i < colors; i++)
		{
			z_color_size[i] = *(u32*)&z_buf[16 + i*4];
		}

		switch (type)
		{
			// zlib
			case 1:
				CZlib zlib;
				z_pbuf += 0x20;
				for (size_t i = 0; i < colors; i++)
				{
					zlib.Decompress(pbuf, color_size, &z_pbuf[4], z_color_size[i]);
					pbuf += color_size;
					z_pbuf += z_color_size[i];
				}
				break;
			// Proprietary compression
			case 3:
				z_pbuf += 0x20;
				for (size_t i = 0; i < colors; i++)
				{
					Decomp(pbuf, color_size, z_pbuf);
					pbuf += color_size;
					z_pbuf += z_color_size[i];
				}
				break;
			// Unknown
			default:
				return false;
		}
	}

	CImage image;
	image.Init(archive, width, height, bpp);
	image.WriteCompoRGBAReverse(&buf[0], buf_len);

	return true;
}

void CAlcot::Decomp(u8* dst, size_t dst_size, const u8* src)
{
	const u8* psrc1 = &src[0x14];
	const u8* psrc2 = psrc1 + *(const u32*)&src[4];
	const u8* psrc3 = psrc2 + *(const u32*)&src[8];
	const u8* dst_end = dst + dst_size;
	u8 code = 0x80;

	while (dst < dst_end)
	{
		if (code == 0)
		{
			psrc1++;
			code = 0x80;
		}

		if (*psrc1 & code)
		{
			const u16 tmp = *(const u16*)psrc2;
			psrc2 += 2;

			const u16 length = (tmp >> 0x0D) + 3;
			if (length > 0)
			{
				const u16 offset = (tmp & 0x1FFF) + 1;
				u8* dst2 = dst - offset;

				std::copy(dst2, dst2 + length, dst);
			}
		}
		else
		{
			const u16 length = *psrc3++ + 1;

			std::copy(psrc3, psrc3 + length, dst);
		}

		code >>= 1;
	}
}

void CAlcot::Decrypt(u8* src, size_t src_size, size_t dst_size)
{
	//                              Kurobane, Kurobane DVD,  TOY‚Â‚ß,   Trip trial,   Trip,       Ramune
	static constexpr u32 games[] = {0xF44387F3, 0xE1B2097A, 0xD153D863, 0xF389842D, 0x1DE71CB9, 0x99E15CB4};

	for (auto game : games)
	{
		u32 key = dst_size ^ game;
		key ^= ((key << 0x0C) | key) << 0x0B;

		const u32 tmp = *(u32*)&src[4] - key;
		if ((tmp & 0x0000FFFF) != 0xDA78)
			continue;

		for (size_t j = 0; j < src_size / sizeof(u32); j++)
		{
			*(u32*)src -= key;
			src += sizeof(u32);
		}
	}
}
