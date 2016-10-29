#pragma once

class CArcFile;

class CZlib
{
public:
	bool Decode(CArcFile* pclArc);
	void DecompressFile(CArcFile* pclArc);
	int Decompress(BYTE* dst, DWORD* dstSize, const BYTE* src, DWORD srcSize);
	int Decompress(BYTE* dst, DWORD dstSize, const BYTE* src, DWORD srcSize);
};
