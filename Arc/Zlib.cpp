#include "StdAfx.h"
#include "../ArcFile.h"
#include "../Image.h"
#include "Zlib.h"

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
int CZlib::Decompress(u8* dst, unsigned long* dst_size, const u8* src, unsigned long src_size)
{
	return uncompress(dst, dst_size, src, src_size);

	/*
	z_stream z;

	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;

	// Initialization
	z.next_in = Z_NULL;
	z.avail_in = 0;
	if (inflateInit(&z) != Z_OK)
	{
		CError error;
		error.Message(nullptr, _T("inflateInit: %s"), z.msg);
		return -1;
	}

	z.next_out = dst; // Output pointer
	z.avail_out = dst_size; // Remaining amount in the output buffer

	z.next_in = src; // Back to the original input pointer position
	z.avail_in = src_size; // Read data

	// Decompress
	inflate(&z, Z_NO_FLUSH);

	// Clean up
	if (inflateEnd(&z) != Z_OK)
	{
		CError error;
		error.Message(NULL, _T("inflateEnd: %s"), z.msg);
		return -1;
	}
*/
}

// Function to expand memory with zlib
int CZlib::Decompress(u8* dst, unsigned long dst_size, const u8* src, unsigned long src_size)
{
	return uncompress(dst, &dst_size, src, src_size);
}
