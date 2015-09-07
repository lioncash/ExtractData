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

	BOOL Decode(CArcFile* pclArc, const void* pvSrc, DWORD dwSrcSize, const YCString& rfclsFileLastName = _T(""));

	BOOL Decomp(void* dst, DWORD dstSize, const void* src, DWORD srcSize);

private:
	BOOL DecompRLE(void* dst, DWORD dstSize, const void* src, DWORD srcSize, BYTE bpp);
};
