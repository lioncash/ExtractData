#pragma once

class CArcFile;

class CZlib
{
public:
	bool Decode(CArcFile* archive);
	void DecompressFile(CArcFile* archive);
	int Decompress(u8* dst, unsigned long* dst_size, const u8* src, unsigned long src_size);
	int Decompress(u8* dst, unsigned long dst_size, const u8* src, unsigned long src_size);
};
