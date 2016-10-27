#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Arc/LZSS.h"
#include "../Arc/Zlib.h"
#include "Ahx.h"
#include "../Image.h"
#include "../Sound/Ogg.h"
#include "Standard.h"

/// Mounting
bool CStandard::Mount(CArcFile* pclArc)
{
	// Get file info
	SFileInfo stFileInfo;
	stFileInfo.name = pclArc->GetArcName();
	stFileInfo.sizeOrg = pclArc->GetArcSize();
	stFileInfo.sizeCmp = stFileInfo.sizeOrg;
	stFileInfo.start = 0;
	stFileInfo.end = stFileInfo.sizeOrg;

	pclArc->AddFileInfo(stFileInfo);

	return true;
}

/// Decoding
bool CStandard::Decode(CArcFile* pclArc)
{
	if (DecodeLZSS(pclArc))
		return true;

	if (DecodeZlib(pclArc))
		return true;

	if (DecodeAhx(pclArc))
		return true;

	if (DecodeImage(pclArc))
		return true;

	if (DecodeOgg(pclArc))
		return true;

	return Extract(pclArc);
}

/// LZSS Decoding
BOOL CStandard::DecodeLZSS(CArcFile* pclArc)
{
	CLZSS clLZSS;

	return clLZSS.Decode(pclArc);
}

/// zlib Decoding
BOOL CStandard::DecodeZlib(CArcFile* pclArc)
{
	CZlib clZlib;

	return clZlib.Decode(pclArc);
}

/// AHX Decoding
BOOL CStandard::DecodeAhx(CArcFile* pclArc)
{
	CAhx clAHX;

	return clAHX.Decode(pclArc);
}

/// Image Decoding
BOOL CStandard::DecodeImage(CArcFile* pclArc)
{
	CImage clImage;

	return clImage.Decode(pclArc);
}

/// Ogg Vorbis Decoding
BOOL CStandard::DecodeOgg(CArcFile* pclArc)
{
	COgg clOgg;

	return clOgg.Decode(pclArc);
}

/// Extraction
bool CStandard::Extract(CArcFile* pclArc)
{
	pclArc->OpenFile();
	pclArc->ReadWrite();
	pclArc->CloseFile();

	return true;
}
