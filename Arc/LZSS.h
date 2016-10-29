#pragma once

class CArcFile;

class CLZSS
{
public:
	bool Decode(CArcFile* pclArc);

	bool Decomp(CArcFile* pclArc, DWORD dwDicSize, DWORD dwDicPtr, DWORD dwLengthOffset);
	bool Decomp(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, DWORD dwDicSize, DWORD dwDicPtr, DWORD dwLengthOffset);
};
