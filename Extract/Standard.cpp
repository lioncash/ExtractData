#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Arc/LZSS.h"
#include "../Arc/Zlib.h"
#include "Ahx.h"
#include "../Image.h"
#include "../Sound/Ogg.h"
#include "Standard.h"

/// Mounting
BOOL CStandard::Mount(CArcFile* pclArc)
{
	// Get file info
	SFileInfo stFileInfo;
	stFileInfo.name = pclArc->GetArcName();
	stFileInfo.sizeOrg = pclArc->GetArcSize();
	stFileInfo.sizeCmp = stFileInfo.sizeOrg;
	stFileInfo.start = 0;
	stFileInfo.end = stFileInfo.sizeOrg;

	pclArc->AddFileInfo(stFileInfo);

	return TRUE;
}

/// Decoding
BOOL CStandard::Decode(CArcFile* pclArc)
{
	if (DecodeLZSS(pclArc))
		return TRUE;

	if (DecodeZlib(pclArc))
		return TRUE;

	if (DecodeAhx(pclArc))
		return TRUE;

	if (DecodeImage(pclArc))
		return TRUE;

	if (DecodeOgg(pclArc))
		return TRUE;

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
BOOL CStandard::Extract(CArcFile* pclArc)
{
	pclArc->OpenFile();
	pclArc->ReadWrite();
	pclArc->CloseFile();

	return TRUE;
}
