#include "StdAfx.h"
#include "Extract/CircusPak.h"

#include "ArcFile.h"
#include "Arc/LZSS.h"
#include "Image.h"
#include "Image/Tga.h"

/// Mounting
///
/// @param archive Archive
///
bool CCircusPak::Mount(CArcFile* archive)
{
	if (MountPakForKujiraCons(archive))
		return true;

	if (MountPakForKujira(archive))
		return true;

	if (MountPakForACDC(archive))
		return true;

	if (MountPakForDCGS(archive))
		return true;

	return false;
}

/// 最終試験くじら-Conservative-向けpakのマウント
///
/// @param archive Archive
///
bool CCircusPak::MountPakForKujiraCons(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pak"))
		return false;

	if (memcmp(archive->GetHeader(), "File Pack 1.0y", 14) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->SeekHed(16);
	archive->ReadU32(&num_files);

	// Decryption
	num_files ^= 0xAAAAAAAA;

	// Get index
	std::vector<PakFileInfoType1> index(num_files);
	archive->SeekCur(4);
	archive->Read(index.data(), sizeof(PakFileInfoType1) * num_files);

	// Get file information
	for (size_t i = 0; i < num_files; i++)
	{
		// Get file name
		char file_name[25];
		for (size_t j = 0; j < 24; j++)
		{
			file_name[j] = index[i].file_name[j] ^ 0xAA;
		}
		file_name[24] = '\0';

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = index[i].offset ^ 0xAAAAAAAA;
		file_info.sizeCmp = index[i].compressed_file_size ^ 0xAAAAAAAA;
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.end = file_info.start + file_info.sizeCmp;

		archive->AddFileInfo(file_info);
	}

	return true;
}

/// 最終試験くじら向けpakのマウント
///
/// @param archive Archive
///
bool CCircusPak::MountPakForKujira(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pak"))
		return false;

	if (memcmp(archive->GetHeader(), "File Pack 1.00", 14) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->SeekHed(16);
	archive->ReadU32(&num_files);

	// Decryption
	num_files ^= 0xAAAAAAAA;

	// Get index
	std::vector<PakFileInfoType2> index(num_files);
	archive->SeekCur(4);
	archive->Read(index.data(), sizeof(PakFileInfoType2) * num_files);

	// Get file information
	for (size_t i = 0; i < num_files; i++)
	{
		// Get file name
		char file_name[33];
		for (size_t j = 0; j < 32; j++)
		{
			file_name[j] = index[i].file_name[j] ^ 0xAA;
		}
		file_name[32] = '\0';

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = index[i].offset ^ 0xAAAAAAAA;
		file_info.sizeCmp = index[i].compressed_file_size ^ 0xAAAAAAAA;
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.end = file_info.start + file_info.sizeCmp;

		archive->AddFileInfo(file_info);
	}

	return true;
}

/// pak mounting for ACDC
///
/// @param archive Archive
///
bool CCircusPak::MountPakForACDC(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pak"))
		return false;

	if (memcmp(archive->GetHeader(), "ACDC_RA", 7) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->SeekHed(8);
	archive->ReadU32(&num_files);

	// Decryption
	num_files ^= 0xA6A6A6A6;

	// Get index
	std::vector<PakFileInfoType2> index(num_files);
	archive->SeekCur(4);
	archive->Read(index.data(), sizeof(PakFileInfoType2) * num_files);

	// Get file information
	for (size_t i = 0; i < num_files; i++)
	{
		// Get filename
		char file_name[33];
		for (size_t j = 0; j < 32; j++)
		{
			file_name[j] = index[i].file_name[j] ^ 0xA6;
		}
		file_name[32] = '\0';

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = index[i].offset ^ 0xA6A6A6A6;
		file_info.sizeCmp = index[i].compressed_file_size ^ 0xA6A6A6A6;
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.end = file_info.start + file_info.sizeCmp;

		archive->AddFileInfo(file_info);
	}

	return true;
}

/// pak mounting for DCGS
///
/// @param archive Archive
///
bool CCircusPak::MountPakForDCGS(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".pak"))
		return false;

	if (memcmp(archive->GetHeader(), "PACK", 4) != 0)
		return false;

	archive->SeekHed(4);

	// Get file count
	u32 num_files;
	archive->ReadU32(&num_files);

	// Get flags
	u32 flags;
	archive->ReadU32(&flags);

	// Get index size
	const size_t index_size = sizeof(PakFileInfoType3) * num_files;

	// Get index
	std::vector<PakFileInfoType3> index(num_files);
	archive->Read(index.data(), index_size);

	// Decode index
	u8* index_ptr = reinterpret_cast<u8*>(index.data());
	for (size_t i = 0; i < index_size; i++)
	{
		index_ptr[i] = (index_ptr[i] << 4) | (index_ptr[i] >> 4);
	}

	// Get file information
	for (size_t i = 0; i < num_files; i++)
	{
		// Add to listview
		SFileInfo file_info;
		file_info.name.Copy(index[i].file_name, sizeof(index[i].file_name));
		file_info.start = index[i].offset;
		file_info.sizeCmp = index[i].compressed_file_size;
		file_info.sizeOrg = index[i].file_size;

		if (file_info.sizeCmp == 0)
		{
			file_info.sizeCmp = file_info.sizeOrg;
		}

		file_info.end = file_info.start + file_info.sizeCmp;

		archive->AddFileInfo(file_info);
	}

	return true;
}

/// Decoding
///
/// @param archive Archive
///
bool CCircusPak::Decode(CArcFile* archive)
{
	if (DecodePakForKujiraCons(archive))
		return true;

	if (DecodePakForKujira(archive))
		return true;

	if (DecodePakForACDC(archive))
		return true;

	if (DecodePakForDCGS(archive))
		return true;

	return false;
}

/// 最終試験くじら-Conservative-向けpakのデコード
///
/// @param archive Archive
///
bool CCircusPak::DecodePakForKujiraCons(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (archive->GetArcExten() != _T(".pak"))
		return false;

	if (memcmp(archive->GetHeader(), "File Pack 1.0y", 14) != 0)
		return false;

	const std::array<u8, 2> key{{
		0xFFU,
		(file_info->format == _T("CS")) ? 1U : 0U
	}};

	if (file_info->format == _T("BMP"))
	{
		// BMP
		std::vector<u8> src(file_info->sizeCmp);
		archive->Read(src.data(), src.size());

		// Decode
		Decrypt1(src.data(), src.size(), key.data());

		// Output
		DecodeBMP(archive, src.data(), src.size());
	}
	else if (file_info->format == _T("CPS"))
	{
		// CPS
		std::vector<u8> src(file_info->sizeCmp);
		archive->Read(src.data(), src.size());

		// Decryption
		Decrypt1(src.data(), src.size(), key.data());
		*reinterpret_cast<u32*>(&src[4]) ^= 0x80701084;

		// Decoding
		DecodeCps(archive, src.data(), src.size());
	}
	else
	{
		// Other
		DecodeEtc(archive, Decrypt2, key.data());
	}

	return true;
}

/// 最終試験くじら向けpakのデコード
///
/// @param archive Archive
///
bool CCircusPak::DecodePakForKujira(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (archive->GetArcExten() != _T(".pak"))
		return false;

	if (memcmp(archive->GetHeader(), "File Pack 1.00", 14) != 0)
		return false;

	const std::array<u8, 2> key{{
		0xFFU,
		file_info->format == _T("CS") ? 1U : 0U
	}};

	if (file_info->format == _T("BMP"))
	{
		// Read
		std::vector<u8> src(file_info->sizeCmp);
		archive->Read(src.data(), src.size());

		// Decryption
		Decrypt1(src.data(), src.size(), key.data());

		// Output
		DecodeBMP(archive, src.data(), src.size());
	}
	else if (file_info->format == _T("CPS"))
	{
		// CPS
		std::vector<u8> src(file_info->sizeCmp);
		archive->Read(src.data(), src.size());

		// Decryption
		Decrypt1(src.data(), src.size(), key.data());
		*reinterpret_cast<u32*>(&src[4]) ^= 0x80701084;

		// Decoding
		DecodeCps(archive, src.data(), src.size());
	}
	else
	{
		// Other
		DecodeEtc(archive, Decrypt2, key.data());
	}

	return true;
}

/// pak decoding for ACDC
///
/// @param archive Archive
///
bool CCircusPak::DecodePakForACDC(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (archive->GetArcExten() != _T(".pak"))
		return false;

	if (memcmp(archive->GetHeader(), "ACDC_RA", 7) != 0)
		return false;

	const std::array<u8, 2> key{{
		0x68U,
		file_info->format == _T("CS") ? 1U : 0U
	}};

	if (file_info->format == _T("BMP"))
	{
		// BMP
		std::vector<u8> src(file_info->sizeCmp);
		archive->Read(src.data(), src.size());

		// Decryption
		Decrypt1(src.data(), src.size(), key.data());

		// Output
		DecodeBMP(archive, src.data(), src.size());
	}
	else if (file_info->format == _T("TGA"))
	{
		// TGA
		std::vector<u8> src(file_info->sizeCmp);
		archive->Read(src.data(), src.size());

		// Decryption
		Decrypt1(src.data(), src.size(), key.data());

		// Output
		CTga tga;
		tga.Decode(archive, src.data(), src.size());
	}
	else if (file_info->format == _T("CPS"))
	{
		// CPS
		const u32 src_size = file_info->sizeCmp;
		std::vector<u8> src(src_size);
		archive->Read(src.data(), src.size());

		// Decryption
		Decrypt1(src.data(), src.size(), key.data());
		*reinterpret_cast<u32*>(&src[4]) ^= 0x0A415FCF;

		if (src_size >= 0x308)
		{
			*reinterpret_cast<u32*>(&src[4]) ^= src[src_size - 1];

			u32 work;
			u32 work2;

			// Replace data
			work = 8 + (src_size - 8) - 0xFF;
			work2 = 8 + ((src_size - 8) >> 9) + 0xFF;

			std::swap(src[work], src[work2]);

			// Replace data
			work = 8 + (src_size - 8) - (0xFF << 1);
			work2 = 8 + (((src_size - 8) >> 9) << 1) + 0xFF;

			std::swap(src[work], src[work2]);
		}

		// Decoding
		DecodeCps(archive, src.data(), src.size());
	}
	else
	{
		// Other
		DecodeEtc(archive, Decrypt2, key.data());
	}

	return true;
}

/// pak decoding for DCGS
///
/// @param archive Archive
///
bool CCircusPak::DecodePakForDCGS(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (archive->GetArcExten() != _T(".pak"))
		return false;

	if (memcmp(archive->GetHeader(), "PACK", 4) != 0)
		return false;

	// Read
	const u32 src_size = file_info->sizeCmp;
	std::vector<u8> src(src_size);
	archive->Read(src.data(), src.size());

	// Decompression
	u8* dst_ptr = src.data();
	u32 dst_size = file_info->sizeOrg;
	std::vector<u8> dst;

	if (src_size != dst_size)
	{
		// Is compressed
		dst.resize(dst_size);

		// Decompression
		CLZSS lzss;
		lzss.Decomp(dst.data(), dst.size(), src.data(), src.size(), 4096, 4078, 3);
		dst_ptr = dst.data();
	}

	// Output
	if (file_info->format == _T("OP2"))
	{
		// Image
		const s32  width      = *reinterpret_cast<s32*>(&dst_ptr[4]);
		const s32  height     = *reinterpret_cast<s32*>(&dst_ptr[8]);
		const u16  bpp        = *reinterpret_cast<u16*>(&dst_ptr[12]);
		const u32 data_offset = *reinterpret_cast<u32*>(&dst_ptr[20]);
		const u32 pallet_size = data_offset - 32;
		const u32 dib_size    = *reinterpret_cast<u32*>(&dst_ptr[24]);

		dst_size = *reinterpret_cast<u32*>(&dst_ptr[28]);

		std::vector<u8> dib(dib_size);

		// Decompression
		CLZSS lzss;
		lzss.Decomp(dib.data(), dib.size(), &dst_ptr[data_offset], dst_size, 4096, 4078, 3);

		// Output
		CImage image;
		image.Init(archive, width, height, bpp, &dst_ptr[32], pallet_size);
		image.WriteReverse(dib.data(), dib.size());
		image.Close();
	}
	else
	{
		// Other
		archive->OpenFile();
		archive->WriteFile(dst_ptr, dst_size);
		archive->CloseFile();
	}

	return true;
}

/// BMP Decoding
///
/// @param archive  Archive
/// @param src      BMP data (to be decoded)
/// @param src_size BMP data size
///
bool CCircusPak::DecodeBMP(CArcFile* archive, const u8* src, size_t src_size)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();
	if (file_info->format != _T("BMP"))
		return false;

	const auto* bmp_file_header = reinterpret_cast<const BITMAPFILEHEADER*>(&src[0]);
	const auto* bmp_info_header = reinterpret_cast<const BITMAPINFOHEADER*>(&src[14]);

	std::vector<u8> dst(src, src + src_size);
	const u32 dst_size = (bmp_info_header->biBitCount == 8) ? (src_size - 54 - 1024) : (src_size - 54);

	CImage image;
	image.Init(archive, dst.data());
	image.Write(dst_size);

	return true;
}

/// CPS Decoding
///
/// @param archive  Archive
/// @param src      CPS data (to be decoded)
/// @param src_size CPS data size
///
bool CCircusPak::DecodeCps(CArcFile* archive, const u8* src, size_t src_size)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("CPS"))
		return false;

	// Ensure output buffer exists
	u32 dst_size = *reinterpret_cast<const u32*>(&src[4]);
	std::vector<u8> dst(dst_size);

	// Decompress
	if (memcmp(src, "CCC0", 4) == 0)
	{
		// CCC0
		DecompCCC0(dst.data(), dst_size, src, src_size);

		const s32 width = *reinterpret_cast<u16*>(&dst[12]);
		const s32 height = *reinterpret_cast<u16*>(&dst[14]);
		const u16 bpp = dst[16];

		// Output
		CImage image;
		image.Init(archive, width, height, bpp);
		image.Write(&dst[18], dst_size - 18);
	}
	else if (memcmp(src, "CCM0", 4) == 0)
	{
		// CCM0
		DecompCCM0(dst.data(), dst_size, src, src_size);

		// Output
		const auto* bmp_file_header = reinterpret_cast<BITMAPFILEHEADER*>(&dst[0]);
		const auto* bmp_info_header = reinterpret_cast<BITMAPINFOHEADER*>(&dst[14]);
		dst_size = (bmp_info_header->biBitCount == 8) ? (dst_size - 54 - 1024) : (dst_size - 54);

		CImage image;
		image.Init(archive, dst.data());
		image.Write(dst_size);
	}
	else
	{
		// Unknown
		archive->OpenFile();
		archive->WriteFile(src, src_size);
	}

	return true;
}

/// Decoding 'Other' data
///
/// @param archive             Archive
/// @param decryption_function Pointer to the decryption function
/// @param key                 Decryption key
///
bool CCircusPak::DecodeEtc(CArcFile* archive, DecryptionFunction decryption_function, const u8* key)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	size_t buffer_size = archive->GetBufSize();
	std::vector<u8> buffer(buffer_size);

	// Generate output files
	if (file_info->format == _T("CS"))
	{
		archive->OpenScriptFile();
	}
	else
	{
		archive->OpenFile();
	}

	for (size_t write_size = 0; write_size < file_info->sizeOrg; write_size += buffer_size)
	{
		// Buffer size adjustment
		archive->SetBufSize(&buffer_size, write_size);

		// Output the decoded data
		archive->Read(buffer.data(), buffer_size);
		decryption_function(buffer.data(), buffer_size, key);
		archive->WriteFile(buffer.data(), buffer_size);
	}

	return true;
}

/// CCC0 Extraction
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      CCC0 data
/// @param src_size CCC0 data size
///
bool CCircusPak::DecompCCC0(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	size_t src_bit_idx = 0x2C * 8;
	size_t dst_idx = 0;

	// Decompression
	for (dst_idx = 0; dst_idx < dst_size; dst_idx += 2)
	{
		u32 index;

		for (index = 0; index < 0x0F; index++)
		{
			if (GetBit(src, &src_bit_idx, 1) == 1)
			{
				break;
			}
		}

		if (index == 0x0F)
		{
			if (GetBit(src, &src_bit_idx, 1) == 0)
			{
				index++;
			}
		}

		if (index != 0)
		{
			*reinterpret_cast<u16*>(&dst[dst_idx]) = reinterpret_cast<const u16*>(src + 8)[index - 1];
		}
		else
		{
			*reinterpret_cast<u16*>(&dst[dst_idx]) = GetBit(src, &src_bit_idx, 16);
		}
	}

	// サイズが奇数の場合1余る

	if (dst_size & 1)
	{
		dst[dst_idx] = src[0x28];
	}

	return true;
}

/// CCM0 Extraction
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      CCM0 data
/// @param src_size CCM0 data size
///
bool CCircusPak::DecompCCM0(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	if (dst_size < 0x80)
	{
		// Uncompressed
		memcpy(dst, &src[8], dst_size);
		return true;
	}

	// Compressed data
	size_t dst_idx = 0;
	for (size_t i = 0; i < 0x80; i++)
	{
		dst[dst_idx++] = src[8 + i] + 0xF0;
	}

	size_t src_bit_idx = 0x88 * 8;
	while (dst_idx < dst_size)
	{
		const u32 flag = GetBit(src, &src_bit_idx, 1);

		// Compressed
		if (flag & 1)
		{
			const u32 back = GetBit(src, &src_bit_idx, 7) + 1;
			const u32 length = GetBit(src, &src_bit_idx, 4) + 2;

			for (size_t i = 0; i < length; i++)
			{
				dst[dst_idx + i] = dst[dst_idx - back + i];
			}

			dst_idx += length;
		}
		else // Uncompressed
		{
			dst[dst_idx++] = GetBit(src, &src_bit_idx, 8);
		}
	}

	return true;
}

/// Get bit sequence
///
/// @param src              Input data
/// @param src_bit_idx      Position of input data (number of bits read is added)
/// @param num_bits_to_read Number of bits to read
///
u32 CCircusPak::GetBit(const u8* src, size_t* src_bit_idx, u32 num_bits_to_read)
{
	u32 data = 0;

	for (size_t i = 0; i < num_bits_to_read; i++)
	{
		data <<= 1;
		data |= (src[*src_bit_idx >> 3] >> (*src_bit_idx & 7)) & 1;
		(*src_bit_idx)++;
	}

	return data;
}

/// Decryption 1
///
/// @param target      Data to be decrypted
/// @param target_size Decryption size
/// @param key         Decryption key
///
void CCircusPak::Decrypt1(u8* target, size_t target_size, const u8* key)
{
	for (size_t i = 0; i < target_size; i++)
	{
		target[i] ^= key[0];
	}
}

/// Decryption 2
///
/// @param target      Data to be decrypted
/// @param target_size Decryption size
/// @param key         Decryption key
///
void CCircusPak::Decrypt2(u8* target, size_t target_size, const u8* key)
{
	for (size_t i = 0; i < target_size; i++)
	{
		target[i] ^= key[0];
		target[i] -= key[1];
	}
}
