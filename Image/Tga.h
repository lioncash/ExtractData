#pragma once

class CTga final
{
public:
	struct STGAHeader
	{
		BYTE btID;
		BYTE btColorMap;
		BYTE btImageType;
		BYTE abtCMInfo[5];
		WORD wOriginX;
		WORD wOriginY;
		WORD wWidth;
		WORD wHeight;
		BYTE btDepth;
		BYTE btParam;
	};

	bool Decode(CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize, const YCString& rfclsFileLastName = _T(""));

	bool Decomp(u8* dst, size_t dst_size, const u8* src, size_t src_size);

private:
	bool DecompRLE(u8* dst, size_t dst_size, const u8* src, size_t src_size, u8 bpp);
};
