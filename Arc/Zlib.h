#pragma once

class CArcFile;

class CZlib
{
public:
	bool Decode(CArcFile* archive);
	void DecompressFile(CArcFile* archive);
	int Decompress(BYTE* dst, DWORD* dstSize, const BYTE* src, DWORD srcSize);
	int Decompress(BYTE* dst, DWORD dstSize, const BYTE* src, DWORD srcSize);
};
