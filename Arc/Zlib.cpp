#include "StdAfx.h"
#include "Arc/Zlib.h"

#include "ArcFile.h"
#include "Common.h"
#include "Image.h"

bool CZlib::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("zlib"))
		return false;

	DecompressFile(archive);

	return true;
}

void CZlib::DecompressFile(CArcFile* archive)
{
	SFileInfo* file_info = archive->GetOpenFileInfo();

	// Ensure buffer
	std::vector<u8> z_buf(file_info->size_cmp);
	std::vector<u8> buf(file_info->size_org);

	// zlib Decompression
	archive->Read(z_buf.data(), z_buf.size());
	Decompress(buf.data(), &file_info->size_org, z_buf.data(), z_buf.size());

	if (lstrcmp(PathFindExtension(file_info->name), _T(".bmp")) == 0)
	{
		CImage image;
		image.Init(archive, buf.data());
		image.Write(file_info->size_org);
	}
	else
	{
		// Output
		archive->OpenFile();
		archive->WriteFile(buf.data(), file_info->size_org);
	}
}

// Function to expand memory with zlib
int CZlib::Decompress(u8* dst, u32* dst_size, const u8* src, u32 src_size)
{
  unsigned long dst_size_tmp = 0;
	const int result = uncompress(dst, &dst_size_tmp, src, src_size);

  *dst_size = static_cast<u32>(dst_size_tmp);
  return result;
}

// Function to expand memory with zlib
int CZlib::Decompress(u8* dst, u32 dst_size, const u8* src, u32 src_size)
{
  unsigned long dst_size_tmp = dst_size;
	return uncompress(dst, &dst_size_tmp, src, src_size);
}
