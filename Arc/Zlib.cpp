#include "stdafx.h"
#include "../ArcFile.h"
#include "../Image.h"
#include "Zlib.h"

BOOL CZlib::Decode(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->format != _T("zlib"))
		return FALSE;

	DecompressFile(pclArc);

	return TRUE;
}

void CZlib::DecompressFile(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	// Ensure buffer
	YCMemory<BYTE> z_buf(pInfFile->sizeCmp);
	YCMemory<BYTE> buf(pInfFile->sizeOrg);

	// zlib Decompression
	pclArc->Read(&z_buf[0], pInfFile->sizeCmp);
	Decompress(&buf[0], &pInfFile->sizeOrg, &z_buf[0], pInfFile->sizeCmp);

	if (lstrcmp(PathFindExtension(pInfFile->name), _T(".bmp")) == 0) {
		CImage image;
		image.Init(pclArc, &buf[0]);
		image.Write(pInfFile->sizeOrg);
	}
	else {
		// Output
		pclArc->OpenFile();
		pclArc->WriteFile(&buf[0], pInfFile->sizeOrg);
	}
}

// Function to expand memory with zlib
int CZlib::Decompress(BYTE* dst, DWORD* dstSize, const BYTE* src, DWORD srcSize)
{
	return uncompress(dst, dstSize, src, srcSize);

	/*
	z_stream z;

	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;

	// Initialization
	z.next_in = Z_NULL;
	z.avail_in = 0;
	if (inflateInit(&z) != Z_OK) {
		CError error;
		error.Message(NULL, _T("inflateInit: %s"), z.msg);
		return -1;
	}

	z.next_out = dst; // Output pointer
	z.avail_out = dstSize; // Remaining amount in the output buffer

	z.next_in = src; // Back to the original input pointer position
	z.avail_in = srcSize; // Read data

	// Decompress
	inflate(&z, Z_NO_FLUSH);

	// Clean up
	if (inflateEnd(&z) != Z_OK) {
		CError error;
		error.Message(NULL, _T("inflateEnd: %s"), z.msg);
		return -1;
	}
*/
}

// Function to expand memory with zlib
int CZlib::Decompress(BYTE* dst, DWORD dstSize, const BYTE* src, DWORD srcSize)
{
	return uncompress(dst, &dstSize, src, srcSize);
}
