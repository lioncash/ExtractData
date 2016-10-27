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

	bool Decomp(void* dst, DWORD dstSize, const void* src, DWORD srcSize);

private:
	bool DecompRLE(void* dst, DWORD dstSize, const void* src, DWORD srcSize, BYTE bpp);
};
