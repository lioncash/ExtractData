#pragma once

#include "../ExtractBase.h"

class CAOS final : public CExtractBase
{
public:
	struct SABMHeader
	{
		BITMAPFILEHEADER stBMPFileHeader; // Bitmap File Header
		BITMAPINFOHEADER stBMPInfoHeader; // Bitmap Info Header
		WORD             wUnknown[2];     // Unknown
		DWORD            dwFrames;        // Number of frames
	};

	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	BOOL DecodeABM(CArcFile* pclArc);
	BOOL DecodeMask(CArcFile* pclArc);
	BOOL DecodeScript(CArcFile* pclArc);

	BOOL DecompABM(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize);
	BOOL DecompScript(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize);

	DWORD CreateHuffmanTable(DWORD* pdwTableOfBit0, DWORD* pdwTableOfBit1, const BYTE* pbtSrc, DWORD* pdwSrcPtr, DWORD* pdwTablePtr, DWORD* pdwCurrentSrc, DWORD* pdwBitShift);
	BOOL DecompHuffman(BYTE* pbtDst, DWORD dwDstSize, const DWORD* pdwTableOfBit0, const DWORD* pdwTableOfBit1, const BYTE* pbtSrc, DWORD dwRoot, DWORD dwCurrentSrc, DWORD dwBitShift);
};
