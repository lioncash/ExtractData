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

	bool Decode(CArcFile* archive, const u8* src, size_t src_size, const YCString& file_last_name = _T(""));

	bool Decomp(u8* dst, size_t dst_size, const u8* src, size_t src_size);

private:
	bool DecompRLE(u8* dst, size_t dst_size, const u8* src, size_t src_size, u8 bpp);
};
