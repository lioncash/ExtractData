#include "StdAfx.h"
#include "../Arc/LZSS.h"
#include "../Arc/Zlib.h"
#include "Ahx.h"
#include "../Image.h"
#include "../Sound/Ogg.h"
#include "Standard.h"

/// Mounting
bool CStandard::Mount(CArcFile* archive)
{
	// Get file info
	SFileInfo file_info;
	file_info.name = archive->GetArcName();
	file_info.sizeOrg = archive->GetArcSize();
	file_info.sizeCmp = file_info.sizeOrg;
	file_info.start = 0;
	file_info.end = file_info.sizeOrg;

	archive->AddFileInfo(file_info);

	return true;
}

/// Decoding
bool CStandard::Decode(CArcFile* archive)
{
	if (DecodeLZSS(archive))
		return true;

	if (DecodeZlib(archive))
		return true;

	if (DecodeAhx(archive))
		return true;

	if (DecodeImage(archive))
		return true;

	if (DecodeOgg(archive))
		return true;

	return Extract(archive);
}

/// LZSS Decoding
bool CStandard::DecodeLZSS(CArcFile* archive)
{
	CLZSS lzss;

	return lzss.Decode(archive);
}

/// zlib Decoding
bool CStandard::DecodeZlib(CArcFile* archive)
{
	CZlib zlib;

	return zlib.Decode(archive);
}

/// AHX Decoding
bool CStandard::DecodeAhx(CArcFile* archive)
{
	CAhx ahx;

	return ahx.Decode(archive);
}

/// Image Decoding
bool CStandard::DecodeImage(CArcFile* archive)
{
	CImage image;

	return image.Decode(archive);
}

/// Ogg Vorbis Decoding
bool CStandard::DecodeOgg(CArcFile* archive)
{
	COgg ogg;

	return ogg.Decode(archive);
}

/// Extraction
bool CStandard::Extract(CArcFile* archive)
{
	archive->OpenFile();
	archive->ReadWrite();
	archive->CloseFile();

	return true;
}
